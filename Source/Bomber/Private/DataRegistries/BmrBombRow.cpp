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

// Finds bomb row data by instigator actor, resolves PlayerTag from its MapComponent or SkeletalMeshComponent
const FBmrBombRow& FBmrBombRow::GetBombRow(const AActor* InInstigator)
{
	static const FBmrBombRow EmptyRowData;
	if (!ensureMsgf(InInstigator, TEXT("ASSERT: [%i] %hs:\n'InInstigator' is not valid!"), __LINE__, __FUNCTION__))
	{
		return EmptyRowData;
	}

	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;
	const APawn* InstigatorPawn = Cast<APawn>(InInstigator);
	const ABmrPlayerState* InstigatorPlayerState = InstigatorPawn ? InstigatorPawn->GetPlayerState<ABmrPlayerState>() : nullptr;

	if (InstigatorPlayerState)
	{
		const FBmrPlayerRow* PlayerRowData = FBmrPlayerRow::GetRowByName(InstigatorPlayerState->GetChosenMeshData().RowName);
		PlayerTag = PlayerRowData ? PlayerRowData->PlayerTag : FBmrPlayerTag::None;
	}
	else if (const UBmrSkeletalMeshComponent* MeshComponent = InInstigator->FindComponentByClass<UBmrSkeletalMeshComponent>())
	{
		// Fallback for non-pawn instigators: e.g. main menu skeletal mesh actor
		PlayerTag = MeshComponent->GetPlayerTag();
	}

	const FBmrBombRow* FoundRow = GetRowByPredicate([&PlayerTag](const FBmrBombRow& Row)
	{
		return Row.PlayerTag == PlayerTag;
	});

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
