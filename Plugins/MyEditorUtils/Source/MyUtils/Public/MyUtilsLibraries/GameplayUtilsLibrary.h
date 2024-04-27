// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "GameplayUtilsLibrary.generated.h"

class USaveGame;

/**
 * Function library with gameplay-related helpers.
 */
UCLASS()
class MYUTILS_API UGameplayUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Abstract method that allows set both static and skeletal meshes to the specified mesh component.
	 * @param MeshComponent The mesh component to set mesh.
	 * @param MeshAsset The mesh asset to set to the mesh component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetMesh(class UMeshComponent* MeshComponent, class UStreamableRenderAsset* MeshAsset);

	/** Returns the first child actor of the specified class.
	  * @param ParentActor The parent actor to search in.
	  * @param ChildActorClass The class of the attached actor to find.
	  * @param bIncludeDescendants If true, also include all attached actors of each attached actor. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static AActor* GetAttachedActorByClass(const AActor* ParentActor, TSubclassOf<AActor> ChildActorClass, bool bIncludeDescendants = false);

	/** Returns the first child actor of the specified class. */
	template <typename T>
	static T* GetAttachedActorByClass(const AActor* ParentActor, bool bIncludeDescendants = false) { return Cast<T>(GetAttachedActorByClass(ParentActor, T::StaticClass(), bIncludeDescendants)); }

	/** Completely removes given save data and creates new empty one.
	 * @param SaveGame The save game to reset.
	 * @param SaveSlotName The name of the save slot.
	 * @param SaveSlotIndex The index of the save slot.
	 * @return The new empty save game object. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static USaveGame* ResetSaveGameData(USaveGame* SaveGame, const FString& SaveSlotName, int32 SaveSlotIndex);
};
