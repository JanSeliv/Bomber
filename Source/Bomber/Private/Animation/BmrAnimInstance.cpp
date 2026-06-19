// Copyright (c) Yevhenii Selivanov

#include "Animation/BmrAnimInstance.h"

// Bomber
#include "Animation/BmrAnimInstanceProxy.h"
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

	ApplyMovementBlendspace();

	if (UBmrSkeletalMeshComponent* OwningMesh = Cast<UBmrSkeletalMeshComponent>(GetOwningComponent()))
	{
		// Re-resolve blend space when owner swaps character on shared skeleton, where engine reuses this instance and skips this init
		OwningMesh->OnMeshDataChanged.AddUniqueDynamic(this, &ThisClass::OnOwnerMeshDataChanged);
	}

	UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();
	GameFeaturesSubsystem.RemoveObserver(this);
	GameFeaturesSubsystem.AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);
}

// Re-resolves movement blend space from current owning mesh row, applied when character mesh swaps on shared skeleton
void UBmrAnimInstance::ApplyMovementBlendspace()
{
	UDalRegistrySubsystem& DalRegistryRef = UDalRegistrySubsystem::Get();

	// Drop previous row listener so repeated swaps do not accumulate pending listeners
	DalRegistryRef.UnbindFromDataRegistryRows(this);

	DalRegistryRef.ListenForDataRegistryRow<FBmrPlayerRow>(this, GetOwnerPlayerRowName(), [this](const FBmrPlayerRow& PlayerRow)
	{
		MovementBlendspace = PlayerRow.IdleWalkRunBlendSpace.Get();
	});
}

// Called when owning mesh swaps to different character
void UBmrAnimInstance::OnOwnerMeshDataChanged_Implementation()
{
	ApplyMovementBlendspace();
}

// Called when animation instance is torn down
void UBmrAnimInstance::NativeUninitializeAnimation()
{
	if (UBmrSkeletalMeshComponent* OwningMesh = Cast<UBmrSkeletalMeshComponent>(GetOwningComponent()))
	{
		OwningMesh->OnMeshDataChanged.RemoveDynamic(this, &ThisClass::OnOwnerMeshDataChanged);
	}

	UGameFeaturesSubsystem* GameFeatureSubsystem = GEngine ? GEngine->GetEngineSubsystem<UGameFeaturesSubsystem>() : nullptr;
	if (GameFeatureSubsystem)
	{
		GameFeatureSubsystem->RemoveObserver(this);
	}

	if (UDalRegistrySubsystem* DalRegistry = UDalRegistrySubsystem::GetDalRegistrySubsystem())
	{
		DalRegistry->UnbindFromDataRegistryRows(this);
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

	// Release the blend spaces cached deeper in the anim proxy and graph nodes, they survive past unload otherwise
	GetProxyOnGameThread<FBmrAnimInstanceProxy>().ResetBlendSpacesInModule(GameFeatureData);
}

// Called by engine when anim instance proxy needs to be allocated
FAnimInstanceProxy* UBmrAnimInstance::CreateAnimInstanceProxy()
{
	return new FBmrAnimInstanceProxy(this);
}

// Returns player Data Registry row name of owning mesh, None when owner is not Bomber mesh
FName UBmrAnimInstance::GetOwnerPlayerRowName() const
{
	const UBmrSkeletalMeshComponent* MeshComponent = Cast<UBmrSkeletalMeshComponent>(GetOwningComponent());
	return MeshComponent ? MeshComponent->GetMeshData().RowName : NAME_None;
}
