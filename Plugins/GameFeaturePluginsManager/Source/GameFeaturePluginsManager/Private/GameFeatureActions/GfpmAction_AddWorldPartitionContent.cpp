// Copyright (c) Yevhenii Selivanov

#include "GameFeatureActions/GfpmAction_AddWorldPartitionContent.h"

#if WITH_EDITOR
// UE
#include "WorldPartition/DataLayer/ExternalDataLayerAsset.h"
#include "WorldPartition/DataLayer/ExternalDataLayerEngineSubsystem.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmAction_AddWorldPartitionContent)

// Called by the Game Features system when the owning feature is registered
void UGfpmAction_AddWorldPartitionContent::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

#if WITH_EDITOR
	const UExternalDataLayerAsset* DataLayerAsset = GetExternalDataLayerAsset();
	if (!ensureMsgf(DataLayerAsset, TEXT("ASSERT: [%i] %hs:\n'DataLayerAsset' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Activate right after the engine registered the content, the order matters: engine generates the streaming object only for layers active at play world init, binding activation to Registered state keeps it alive across the whole session so runtime map switching never loses it
	UExternalDataLayerEngineSubsystem::Get().ActivateExternalDataLayerAsset(DataLayerAsset, this);
#endif // WITH_EDITOR
}

// Called by the Game Features system when the owning feature transitions to Active
void UGfpmAction_AddWorldPartitionContent::OnGameFeatureActivating()
{
#if !WITH_EDITOR
	Super::OnGameFeatureActivating();
#endif // !WITH_EDITOR
	// Editor builds drive External Data Layer injection separately for runtime map switching, so the engine default is intentionally suppressed there
}

// Called by the Game Features system when the owning feature is leaving the Active state
void UGfpmAction_AddWorldPartitionContent::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
#if !WITH_EDITOR
	Super::OnGameFeatureDeactivating(Context);
#endif // !WITH_EDITOR
}