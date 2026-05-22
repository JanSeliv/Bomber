// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "Components/GameFrameworkComponentManager.h" // FComponentRequestHandle
#include "GameplayTagContainer.h" // FGameplayTagContainer
#include "UObject/SoftObjectPtr.h" // TSoftClassPtr

#include "GfpmAction_AddLooseGameplayTags.generated.h"

/**
 * Per-world granted-tag tracking for the shared UGfpmAction_AddLooseGameplayTags instance.
 * One action instance services every world (editor preview, PIE server and clients), so granted-tag state is bucketed per world to keep each world's activation independent.
 */
struct FGfpmLooseTagsWorldData
{
	/** Extension request handles registered for this world, populated for game worlds only. */
	TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;

	/** Actors this action granted tags to in this world. */
	TSet<TWeakObjectPtr<AActor>> TaggedActors;

	/** Per-ASC generic tag-event handles for this world, populated only when bIsExclusiveTag is true. */
	TMap<TWeakObjectPtr<class UAbilitySystemComponent>, FDelegateHandle> ExclusiveTagSubscriptions;
};

/**
 * Game Feature action that grants loose gameplay tags to the Ability System Component owned by an actor of a specified class while the feature is Active.
 * Lets one feature activation chain into other tag-driven features by adding tags onto the world ASC. Subscribes to UGameFrameworkComponentManager extension events for actors of the configured class in play worlds.
 */
UCLASS(DisplayName = "GFPM Add Loose Gameplay Tags")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmAction_AddLooseGameplayTags final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/** Owner of Ability System Component; the action subscribes to receivers of this class registered via UGameFrameworkComponentManager and resolves the ASC through IAbilitySystemInterface. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "[Game Feature Plugins Manager]", meta = (AllowAbstract))
	TSoftClassPtr<AActor> OwnerActor = nullptr;

	/** Tags granted to the world ASC while the feature is Active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "[Game Feature Plugins Manager]")
	FGameplayTagContainer LooseTags = FGameplayTagContainer::EmptyContainer;

	/** When true, own LooseTags are removed from the ASC if any other child tag sharing the same direct parent is applied
	 * E.g. LooseTags is 'Map.A', it will self-remove when 'Map.B' is added by anything else.
	 * Is useful to unload own tag-driven feature if another one became loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "[Game Feature Plugins Manager]")
	bool bIsExclusiveTag = false;

	/** Adds LooseTags to actor's ASC and remembers it for cleanup on deactivation. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void GrantTagsTo(AActor* Actor);

	/** Removes LooseTags from actor's ASC and stops tracking it. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void RevokeTagsFrom(AActor* Actor);

protected:
	/** Called by the Game Features system when the owning feature transitions to Active. */
	virtual void OnGameFeatureActivated() override;

	/** Called by the Game Features system when the owning feature is leaving the Active state. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

#if WITH_EDITOR
	/** Reports configuration errors to the editor's Data Validation system: required properties left unset. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	/*********************************************************************************************
	 * Internal
	 ********************************************************************************************* */
protected:
	/** Routes play worlds through GFCM extension handler. */
	void RegisterForWorld(UWorld* World);

	/** Routes UGameFrameworkComponentManager extension events to add or remove tag operations. */
	void OnReceiverExtensionEvent(AActor* Actor, FName EventName);

	/** Hook for game instances starting up while the feature is Active. */
	void OnGameInstanceStarted(UGameInstance* GameInstance);

	/** Hook for worlds that initialize while the feature is Active; covers built-in features that activated before any world existed. */
	void OnPostWorldInit(UWorld* World, struct FWorldInitializationValues WorldInitializationValues);

	/** Removes the granted tags from every tracked actor across all worlds. */
	void RevokeAllTrackedTags();

	/** Removes the granted tags and clears handles for a single world's tracked data. */
	void RevokeWorldData(FGfpmLooseTagsWorldData& WorldData);

	/** Hook for any ASC tag count change; removes own tags when another child tag sharing the same direct parent as one of LooseTags is added. */
	void OnAnyExclusiveAscTagChanged(FGameplayTag ChangedTag, int32 NewCount, TWeakObjectPtr<AActor> WeakActor);

	/** Handle for the FWorldDelegates::OnStartGameInstance binding. */
	FDelegateHandle OnStartGameInstanceHandle;

	/** Handle for the FWorldDelegates::OnPostWorldInitialization binding. */
	FDelegateHandle OnPostWorldInitHandle;

	/** Per-world granted-tag tracking, this action is one shared instance servicing every world. */
	TMap<TWeakObjectPtr<class UWorld>, FGfpmLooseTagsWorldData> PerWorldData;
};
