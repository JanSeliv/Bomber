// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "GameFeatureAction.h"
#include "GameFeatureAction_AddComponents.h" // FGameFeatureComponentEntry
#include "GameFeaturesSubsystem.h" // FGameFeatureStateChangeContext

#include "GfpmAction_AddComponents.generated.h"

/**
 * World-scoped replacement for engine AddComponents action: wraps every per-world iteration in scope guard so synchronously-fired component BeginPlay lands in correct world's pinned context.
 * Avoids context-leak that engine action causes in PIE multiplayer when caller pins global context across all world-context iterations.
 */
UCLASS(DisplayName = "GFPM Add Components")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmAction_AddComponents final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/** When owning Game Feature Plugin transitions into Active state. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** When owning Game Feature Plugin transitions out of Active state. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

#if WITH_EDITORONLY_DATA
	/** When cooker gathers asset bundle data for owning plugin. */
	virtual void AddAdditionalAssetBundleData(struct FAssetBundleData& AssetBundleData) override;
#endif // WITH_EDITORONLY_DATA
#if WITH_EDITOR
	/** When editor validates Game Feature Data asset that owns this action. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;

	/** When ComponentList entry is added in editor. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	/** Mirrors engine ComponentList schema so authored entries round-trip through asset editor.
	 * BP exposure skipped, FGameFeatureComponentEntry uses bitfield uint8 members not supported by Blueprint reflection. */
	UPROPERTY(EditAnywhere, Category = "[Game Feature Plugins Manager]", meta = (TitleProperty = "{ActorClass} -> {ComponentClass}"))
	TArray<FGameFeatureComponentEntry> ComponentList;

private:
	struct FGfpmContextHandles
	{
		FDelegateHandle GameInstanceStartHandle;
		/** Component request handles bucketed by FWorldContext::ContextHandle so deactivation can scope-guard per-world handle release. */
		TMap<FName, TArray<TSharedPtr<struct FComponentRequestHandle>>> ComponentRequestHandlesByWorld;
	};

	/** Per-world component request enqueue against world's framework component manager. */
	void AddToWorld(const struct FWorldContext& WorldContext, FGfpmContextHandles& Handles);

	/** When GameInstance starts after this action's activation. */
	void HandleGameInstanceStart(class UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	TMap<FGameFeatureStateChangeContext, FGfpmContextHandles> ContextHandles;
};
