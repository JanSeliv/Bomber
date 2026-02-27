// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrBombDataAsset.h"

// Bomber
#include "Components/BmrMapComponent.h"
#include "Components/BmrSkeletalMeshComponent.h"
#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBombDataAsset)

// Default constructor
UBmrBombDataAsset::UBmrBombDataAsset()
{
	ActorType = EAT::Bomb;
	RowClass = UBmrBombRow::StaticClass();
}

// Returns the bomb data asset
const UBmrBombDataAsset& UBmrBombDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Returns associated bomb row by associated instigator actor (e.g: Fori character -> Third (Forest) row)
const UBmrBombRow* UBmrBombDataAsset::GetBombRow(const AActor* InInstigator) const
{
	if (!ensureMsgf(InInstigator, TEXT("ASSERT: [%i] %hs:\n'InInstigator' is not valid!"), __LINE__, __FUNCTION__))
	{
		return nullptr;
	}

	EBmrLevelType LevelType = EBmrLevelType::None;
	if (const UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(InInstigator))
	{
		const UBmrLevelActorRow* MeshRow = MapComponent->GetMeshRow();
		LevelType = MeshRow ? MeshRow->LevelType : ELT::None;
	}
	else if (const UBmrSkeletalMeshComponent* MeshComponent = InInstigator->FindComponentByClass<UBmrSkeletalMeshComponent>())
	{
		LevelType = MeshComponent->GetAssociatedLevelType();
	}
	return GetRowByLevelType<UBmrBombRow>(LevelType);
}
