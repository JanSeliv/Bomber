// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Editor/UnrealEdEngine.h"
//---
#include "MyUnrealEdEngine.generated.h"

/**
 * Extends the Unreal Editor Engine class
 * to provide own singleton objects for editor clients in multiplayer.
 */
UCLASS(Transient)
class BOMBEREDITOR_API UMyUnrealEdEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	/** Returns this Unreal Editor Engine object. */
	static const UMyUnrealEdEngine& Get();

	/** Returns overall number of all such singletons of editor clients. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetClientSingletonsNum() const { return ClientSingletonsInternal.Num(); }

	/** Returns singleton of the editor client by specified player index.
	 * @see UMyUnrealEdEngine::ClientSingletonsInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	UObject* GetClientSingleton(int32 Index) const;

	/** Returns singleton of the editor client by specified player index. */
	template <typename T>
	static FORCEINLINE T* GetClientSingleton(int32 Index) { return Cast<T>(Get().GetClientSingleton(Index)); }

protected:
	/** Contains singletons for editor clients.  */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> ClientSingletonsInternal;

	/** Creates a new Play in Editor instance (which may be in a new process if not running under one process. */
	virtual void CreateNewPlayInEditorInstance(FRequestPlaySessionParams& InRequestParams, const bool bInDedicatedInstance, const EPlayNetMode InNetMode) override;

	/** Create new singleton for the editor client if is needed. */
	void AddClientSingleton();
};
