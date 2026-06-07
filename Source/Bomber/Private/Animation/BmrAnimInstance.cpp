// Copyright (c) Yevhenii Selivanov

#include "Animation/BmrAnimInstance.h"

// Bomber
#include "Components/BmrSkeletalMeshComponent.h"
#include "DataRegistries/BmrPlayerRow.h"

// GFPM
#include "GfpmUtils.h"

// DAL
#include "DalRegistrySubsystem.h"

// UE
#include "Animation/BlendSpace1D.h"
#include "GameFeaturesSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrAnimInstance)

// Called once when animation instance is created (new skeletal mesh is set)
void UBmrAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	const FName RowName = GetOwnerPlayerRowName();
	UDalRegistrySubsystem::Get().ListenForDataRegistryRow<FBmrPlayerRow>(this, RowName, [this](const FBmrPlayerRow& PlayerRow)
	{
		MovementBlendspace = PlayerRow.IdleWalkRunBlendSpace.Get();
	});

	UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();
	GameFeaturesSubsystem.RemoveObserver(this);
	GameFeaturesSubsystem.AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);
}

// Called when animation instance is torn down
void UBmrAnimInstance::NativeUninitializeAnimation()
{
	UGameFeaturesSubsystem* GameFeatureSubsystem = GEngine ? GEngine->GetEngineSubsystem<UGameFeaturesSubsystem>() : nullptr;
	if (GameFeatureSubsystem)
	{
		GameFeatureSubsystem->RemoveObserver(this);
	}

	if (UDalRegistrySubsystem* DalRegistry = UDalRegistrySubsystem::GetDalRegistrySubsystem())
	{
		DalRegistry->UnbindFromDataRegistryRow(this, GetOwnerPlayerRowName());
	}

	MovementBlendspace = nullptr;

	Super::NativeUninitializeAnimation();
}

// Called prior to deactivating game feature plugin, when its content is about to be released
void UBmrAnimInstance::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL)
{
	checkf(GameFeatureData, TEXT("ERROR: [%i] %hs:\n'GameFeatureData' is null!"), __LINE__, __FUNCTION__);

	// Release cached blend space when it belongs to the deactivating plugin, so unload does not report it as leaked
	if (UGfpmUtils::IsInGameFeatureModule(MovementBlendspace, GameFeatureData))
	{
		MovementBlendspace = nullptr;
	}
}

// Returns player Data Registry row name of owning mesh, None when owner is not Bomber mesh
FName UBmrAnimInstance::GetOwnerPlayerRowName() const
{
	const UBmrSkeletalMeshComponent* MeshComponent = Cast<UBmrSkeletalMeshComponent>(GetOwningComponent());
	return MeshComponent ? MeshComponent->GetMeshData().RowName : NAME_None;
}
