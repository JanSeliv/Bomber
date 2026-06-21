// Copyright (c) Yevhenii Selivanov

#pragma once

#include "WorldPartition/DataLayer/ExternalDataLayerInjectionPolicy.h"

#include "GfpmExternalDataLayerInjectionPolicy.generated.h"

/**
 * Keeps game feature plugin External Data Layers out of project cook, so its host world generates no cells for them and each plugin ships as separate mod.
 */
UCLASS()
class UGfpmExternalDataLayerInjectionPolicy : public UExternalDataLayerInjectionPolicy
{
	GENERATED_BODY()

#if WITH_EDITOR
public:
	/** When cook decides whether given External Data Layer injects into its host world. */
	virtual bool CanInject(const UWorld* InWorld, const UExternalDataLayerAsset* InExternalDataLayerAsset, const UObject* InClient, FText* OutFailureReason = nullptr) const override;
#endif // WITH_EDITOR
};
