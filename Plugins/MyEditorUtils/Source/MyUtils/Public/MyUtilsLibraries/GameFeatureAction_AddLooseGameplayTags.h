// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFeatureAction.h"

// UE
#include "Components/GameFrameworkComponentManager.h" // FComponentRequestHandle
#include "GameplayTagContainer.h" // FGameplayTagContainer
#include "UObject/SoftObjectPtr.h" // TSoftClassPtr

#include "GameFeatureAction_AddLooseGameplayTags.generated.h"

/**
 * Game Feature action that grants loose gameplay tags to the Ability System Component owned by an actor of a specified class while the feature is Active.
 * Lets one feature activation chain into other tag-driven features by adding tags onto the world ASC, the action subscribes to UGameFrameworkComponentManager extension events for actors of the configured class in play worlds, and listens for spawns and walks already-loaded actors in the editor world, so chained features apply regardless of when the feature activates relative to world load.
 */
UCLASS(meta = (DisplayName = "Add Loose Gameplay Tags"))
class MYUTILS_API UGameFeatureAction_AddLooseGameplayTags final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	/** Owner of Ability System Component; the action subscribes to receivers of this class registered via UGameFrameworkComponentManager and resolves the ASC through IAbilitySystemInterface. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags", meta = (AllowAbstract))
	TSoftClassPtr<AActor> OwnerActor = nullptr;

	/** Tags granted to the world ASC while the feature is Active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer LooseTags = FGameplayTagContainer::EmptyContainer;

	/** When true, own LooseTags are removed from the ASC if any other child tag sharing the same direct parent is applied
	 * E.g. LooseTags is 'Map.A', it will self-remove when 'Map.B' is added by anything else.
	 * Is useful to unload own tag-driven feature if another one became loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	bool bIsExclusiveTag = false;

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
	/** Single dispatch entry per world: routes play worlds through the GFCM extension handler and editor worlds through receiver enumeration plus spawn listening. */
	void RegisterForWorld(UWorld* World);

	/** Routes UGameFrameworkComponentManager extension events to add or remove tag operations. */
	void OnReceiverExtensionEvent(AActor* Actor, FName EventName);

	/** Hook for game instances starting up while the feature is Active. */
	void OnGameInstanceStarted(UGameInstance* GameInstance);

	/** Hook for worlds that initialize while the feature is Active; covers built-in features that activated before any world existed. */
	void OnPostWorldInit(UWorld* World, struct FWorldInitializationValues WorldInitializationValues);

	/** Adds LooseTags to the actor's ASC and remembers the actor for cleanup on deactivation. */
	void GrantTagsTo(AActor* Actor);

	/** Removes LooseTags from the actor's ASC and stops tracking it. */
	void RevokeTagsFrom(AActor* Actor);

	/** Removes the granted tags from every tracked actor and clears all per-world handles. */
	void RevokeAllTrackedTags();

	/** Hook for any ASC tag count change; removes own tags when another child tag sharing the same direct parent as one of LooseTags is added. */
	void OnAnyExclusiveAscTagChanged(FGameplayTag ChangedTag, int32 NewCount, TWeakObjectPtr<AActor> WeakActor);

	/** Handle for the FWorldDelegates::OnStartGameInstance binding. */
	FDelegateHandle OnStartGameInstanceHandle;

	/** Handle for the FWorldDelegates::OnPostWorldInitialization binding. */
	FDelegateHandle OnPostWorldInitHandle;

	/** Per-world extension request handles, kept alive for the duration of the feature's Active state. */
	TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;

	/** Actors the action has granted tags to, used to remove them on deactivation. */
	TSet<TWeakObjectPtr<AActor>> TaggedActors;

	/** Per-ASC generic tag-event handle kept alive while own LooseTags are granted; populated only when bIsExclusiveTag is true. */
	TMap<TWeakObjectPtr<class UAbilitySystemComponent>, FDelegateHandle> ExclusiveTagSubscriptions;

#if WITH_EDITOR
	/** Receiver hook for actors spawned into editor worlds while the feature is Active. */
	void OnActorSpawnedInEditorWorld(AActor* SpawnedActor);

	/** Hook for editor map-load completion; the persistent level's actors are loaded by this point, so a re-walk can catch level-loaded receivers that never go through SpawnActor. */
	void OnEditorMapOpened(const FString& Filename, bool bAsTemplate);

	/** Per-editor-world spawn-listener handles, kept alive while the editor world exists. */
	TMap<TWeakObjectPtr<UWorld>, FDelegateHandle> EditorActorSpawnedHandles;

	/** Handle for the FEditorDelegates::OnMapOpened binding. */
	FDelegateHandle OnEditorMapOpenedHandle;
#endif
};
