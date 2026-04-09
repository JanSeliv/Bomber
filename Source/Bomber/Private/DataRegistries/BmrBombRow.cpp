// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrBombRow.h"

// Bomber
#include "Components/BmrMapComponent.h"
#include "Components/BmrSkeletalMeshComponent.h"
#include "DataRegistries/BmrPlayerRow.h"

// UE
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBombRow)

// Finds bomb row data by instigator actor, resolves level type from its MapComponent or SkeletalMeshComponent
const FBmrBombRow& FBmrBombRow::GetBombRow(const AActor* InInstigator)
{
	static const FBmrBombRow EmptyRowData;
	if (!ensureMsgf(InInstigator, TEXT("ASSERT: [%i] %hs:\n'InInstigator' is not valid!"), __LINE__, __FUNCTION__))
	{
		return EmptyRowData;
	}

	EBmrLevelType LevelType = EBmrLevelType::None;
	const UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(InInstigator);
	const UBmrSkeletalMeshComponent* MeshComponent = !MapComponent
	                                                     ? InInstigator->FindComponentByClass<UBmrSkeletalMeshComponent>()
	                                                     : nullptr;

	if (MeshComponent)
	{
		LevelType = MeshComponent->GetAssociatedLevelType();
	}
	else if (MapComponent)
	{
		const FBmrPlayerRow* PlayerRowData = FBmrPlayerRow::GetRowByName(MapComponent->GetReplicatedMeshData().RowName);
		LevelType = PlayerRowData ? PlayerRowData->LevelType : EBmrLevelType::None;
	}

	const FBmrBombRow* FoundRow = GetRowByLevelType(LevelType);
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
