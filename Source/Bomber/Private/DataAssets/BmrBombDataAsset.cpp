// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BombDataAsset.h"

// Bomber
#include "Components/MapComponent.h"
#include "Components/MySkeletalMeshComponent.h"
#include "DataAssets/DataAssetsContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BombDataAsset)

// Default constructor
UBombDataAsset::UBombDataAsset()
{
	ActorTypeInternal = EAT::Bomb;
	RowClassInternal = UBombRow::StaticClass();
}

// Returns the bomb data asset
const UBombDataAsset& UBombDataAsset::Get()
{
	return UDataAssetsContainer::GetLevelActorDataAssetChecked<ThisClass>();
}

// Returns associated bomb row by associated instigator actor (e.g: Fori character -> Third (Forest) row)
const UBombRow* UBombDataAsset::GetBombRow(const AActor* InInstigator) const
{
	if (!ensureMsgf(InInstigator, TEXT("ASSERT: [%i] %hs:\n'InInstigator' is not valid!"), __LINE__, __FUNCTION__))
	{
		return nullptr;
	}

	ELevelType LevelType = ELevelType::None;
	if (const UMapComponent* MapComponent = UMapComponent::GetMapComponent(InInstigator))
	{
		const ULevelActorRow* MeshRow = MapComponent->GetMeshRow();
		LevelType = MeshRow ? MeshRow->LevelType : ELT::None;
	}
	else if (const UMySkeletalMeshComponent* MeshComponent = InInstigator->FindComponentByClass<UMySkeletalMeshComponent>())
	{
		LevelType = MeshComponent->GetAssociatedLevelType();
	}
	return GetRowByLevelType<UBombRow>(LevelType);
}
