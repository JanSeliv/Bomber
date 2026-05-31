// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrBombRow.h"

// Bomber
#include "Components/BmrSkeletalMeshComponent.h"
#include "DataRegistries/BmrPlayerRow.h"
#include "GameFramework/BmrPlayerState.h"

// UE
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBombRow)

// Returns bomb row matching instigator's character, or shared Default row when none resolves
const FBmrBombRow& FBmrBombRow::GetBombRow(const AActor* InInstigator)
{
	static const FBmrBombRow EmptyRowData;
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;
	const UBmrSkeletalMeshComponent* MeshComponent = InInstigator ? InInstigator->FindComponentByClass<UBmrSkeletalMeshComponent>() : nullptr;
	if (MeshComponent)
	{
		PlayerTag = MeshComponent->GetPlayerTag();
	}

	// Fallback to Player State chosen mesh data for instigators without skeletal mesh component
	const APawn* InstigatorPawn = Cast<APawn>(InInstigator);
	const ABmrPlayerState* InstigatorPlayerState = InstigatorPawn ? InstigatorPawn->GetPlayerState<ABmrPlayerState>() : nullptr;
	if (!PlayerTag.IsValid()
	    && InstigatorPlayerState)
	{
		const FBmrPlayerRow* PlayerRowData = FBmrPlayerRow::GetRowByName(InstigatorPlayerState->GetChosenMeshData().RowName);
		PlayerTag = PlayerRowData ? PlayerRowData->PlayerTag : FBmrPlayerTag::None;
	}

	// Match resolved character, otherwise fall back to shared Default-character row so environmental or AI bombs without specific placer still receive valid visual
	const FBmrBombRow* FoundRow = GetRowByPredicate([&PlayerTag](const FBmrBombRow& Row)
	{
		return Row.PlayerTag == PlayerTag;
	});
	if (!FoundRow
	    && PlayerTag != FBmrPlayerTag::Default)
	{
		FoundRow = GetRowByPredicate([](const FBmrBombRow& Row)
		{
			return Row.PlayerTag == FBmrPlayerTag::Default;
		});
	}

	return FoundRow ? *FoundRow : EmptyRowData;
}

// Gathers all unique bomb materials from Data Registry
void FBmrBombRow::GetAllBombMaterials(TArray<UMaterialInterface*>& OutMaterials)
{
	OutMaterials.Reset();
	ForEachRow([&OutMaterials](const FBmrBombRow& Row)
	{
		if (UMaterialInterface* MaterialIt = Row.Material.Get())
		{
			OutMaterials.AddUnique(MaterialIt);
		}
	});
}

// Returns the number of unique bomb materials from Data Registry
int32 FBmrBombRow::GetBombMaterialsNum()
{
	TArray<UMaterialInterface*> BombMaterials;
	GetAllBombMaterials(BombMaterials);
	return BombMaterials.Num();
}

// Returns the bomb material by specified index from Data Registry, or nullptr
UMaterialInterface* FBmrBombRow::GetBombMaterial(int32 Index)
{
	TArray<UMaterialInterface*> BombMaterials;
	GetAllBombMaterials(BombMaterials);
	return BombMaterials.IsValidIndex(Index) ? BombMaterials[Index] : nullptr;
}
