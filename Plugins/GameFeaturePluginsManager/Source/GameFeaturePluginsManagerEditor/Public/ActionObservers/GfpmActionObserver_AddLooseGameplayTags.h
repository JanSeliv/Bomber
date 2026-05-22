// Copyright (c) Yevhenii Selivanov

#pragma once

#include "ActionObservers/GfpmActionObserver_Base.h"

#include "GfpmActionObserver_AddLooseGameplayTags.generated.h"

/**
 * Editor-only observer that drives loose-tag application onto actors spawned into editor worlds.
 * Keeps tags applied across editor map loads for the plugin whose observed action it was created for.
 */
UCLASS()
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmActionObserver_AddLooseGameplayTags final : public UGfpmActionObserver_Base
{
	GENERATED_BODY()

public:
	/** Identifies the action type this observer handles. */
	virtual TSubclassOf<class UGameFeatureAction> GetObservedActionClass() const override;

	/** When owning plugin finished activating, after its actions ran. */
	virtual void OnGameFeatureActivated() override;

	/** When owning plugin begins deactivating. */
	virtual void OnGameFeatureDeactivating() override;

protected:
	/** FEditorDelegates::OnMapOpened subscription handle */
	FDelegateHandle OnEditorMapOpenedHandle;

	/** Per-editor-world spawn-listener handles, kept alive while the editor world exists */
	TMap<TWeakObjectPtr<class UWorld>, FDelegateHandle> EditorActorSpawnedHandles;

	/** Registers spawn listener and grants tags to already-loaded actors of given editor world */
	void RegisterForEditorWorld(class UWorld* World);

	/** Walks every editor world to register spawn listeners and grant tags to already-loaded actors */
	void WalkAllEditorWorlds();

	/** When an actor is spawned into an editor world */
	void OnActorSpawnedInEditorWorld(class AActor* SpawnedActor);

	/** When an editor map finishes loading */
	void OnEditorMapOpened(const FString& Filename, bool bAsTemplate);
};
