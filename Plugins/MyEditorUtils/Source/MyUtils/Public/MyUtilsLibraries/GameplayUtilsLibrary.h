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
	/*********************************************************************************************
	 * Multiplayer Helpers
	 ********************************************************************************************* */
public:
	/** Returns true if this instance has authority. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsServer();

	/** Returns true if any client is connected to the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static FORCEINLINE bool IsMultiplayerGame() { return GetPlayersInMultiplayerNum() > 1; }

	/** Returns amount of players (host + clients) playing this game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetPlayersInMultiplayerNum();

	/*********************************************************************************************
	 * Actor Helpers
	 ********************************************************************************************* */
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

	/** Is useful for animating actor's transform from values stored in the Curve Table.
	 * Applies the transform in local space from a given Curve Table to the specified actor over time (seconds).
	 * @param InActor The actor to which the evaluated transform will be applied.
	 * @param CenterWorldTransform The initial transform of the actor to apply animation on it.
	 * @param CurveTable The Curve Table with next rows: "LocationX", "LocationY", "LocationZ", "RotationPitch", "RotationYaw", "RotationZ", "RotationRoll", "ScaleX", "ScaleY", "ScaleZ".
	 * @param TotalSecondsSinceStart The time in seconds since the animation started, used to evaluate the Curve Table.
	 * @return Returns false if Curve Table is finished or can not be evaluated (no row found, etc.). */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "CenterWorldTransform"))
	static bool ApplyTransformFromCurveTable(AActor* InActor, const FTransform& CenterWorldTransform, class UCurveTable* CurveTable, float TotalSecondsSinceStart);

	/*********************************************************************************************
	 * Save Helpers
	 ********************************************************************************************* */
public:
	/** Completely removes given save data and creates new empty one.
	 * @param SaveGame The save game to reset.
	 * @param SaveSlotName The name of the save slot.
	 * @param SaveSlotIndex The index of the save slot.
	 * @return The new empty save game object. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static USaveGame* ResetSaveGameData(USaveGame* SaveGame, const FString& SaveSlotName, int32 SaveSlotIndex);

	/** Returns true if given object's config properties can be saved to the config file.
	 * Is useful for validations to avoid unexpected behavior in builds.
	 * @param Object The object to check. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool CanSaveConfig(const UObject* Object);

	/** Saves the given object's config properties to the config file.
	 * Is useful because of its validation to avoid unexpected behavior in builds.
	 * @param Object The object to save. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SaveConfig(UObject* Object);
};
