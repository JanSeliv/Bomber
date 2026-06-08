// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Animation/AnimInstance.h"

// UE
#include "GameFeatureStateChangeObserver.h"

#include "BmrAnimInstance.generated.h"

/**
 * Base animation instance for Bomber animation blueprint.
 */
UCLASS()
class BOMBER_API UBmrAnimInstance : public UAnimInstance
    , public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	/** Idle/walk/run movement blend space of current character, read by Anim Graph. Resolved from active player Data Registry row, null when no row available or its source plugin unloaded. */
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Transient, Category = "[Bomber]")
	TObjectPtr<class UBlendSpace1D> MovementBlendspace = nullptr;

protected:
	/** Called once when animation instance is created (new skeletal mesh is set). */
	virtual void NativeInitializeAnimation() override;

	/** Called when animation instance is torn down. */
	virtual void NativeUninitializeAnimation() override;

	/** Called prior to deactivating game feature plugin, when its content is about to be released. */
	virtual void OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL) override;

	/** Returns player Data Registry row name of owning mesh, None when owner is not Bomber mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (BlueprintProtected))
	FName GetOwnerPlayerRowName() const;

	/** Called by engine when anim instance proxy needs to be allocated. */
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};
