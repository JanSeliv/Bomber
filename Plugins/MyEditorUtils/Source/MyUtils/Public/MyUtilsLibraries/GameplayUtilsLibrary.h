// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameplayUtilsLibrary.generated.h"

/**
 * Function library with gameplay-related helpers.
 */
UCLASS()
class MYUTILS_API UGameplayUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Actor Helpers
	 ********************************************************************************************* */
public:
	/** Abstract getter that allows to obtain the static or skeletal mesh from given mesh component (base class of both).
	 * @param MeshComponent The mesh component to get mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UStreamableRenderAsset* GetMesh(const class UMeshComponent* MeshComponent);

	/** Alternative code-only getter, e.g: UStaticMesh* StaticMesh = UGameplayUtilsLibrary::GetMesh<UStaticMesh>(MeshComponent); */
	template <typename T>
	static T* GetMesh(const UMeshComponent* MeshComponent) { return Cast<T>(GetMesh(MeshComponent)); }

	/** Abstract method that allows set both static and skeletal meshes to the specified mesh component by its base class.
	 * @param MeshComponent The mesh component to set mesh.
	 * @param MeshAsset The mesh asset to set to the mesh component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetMesh(class UMeshComponent* MeshComponent, class UStreamableRenderAsset* MeshAsset);

	/** Returns the first child actor of the specified class.
	 * @param ParentActor The parent actor to search in.
	 * @param ChildActorClass The class of the attached actor to find.
	 * @param bIncludeDescendants If true, also include all attached actors of each attached actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static AActor* GetAttachedActorByClass(const AActor* ParentActor, TSubclassOf<AActor> ChildActorClass, bool bIncludeDescendants = false);

	/** Returns the first child actor of the specified class. */
	template <typename T>
	static T* GetAttachedActorByClass(const AActor* ParentActor, bool bIncludeDescendants = false) { return Cast<T>(GetAttachedActorByClass(ParentActor, T::StaticClass(), bIncludeDescendants)); }

	/** Extracts transform values from a Curve Table at a specific time
	 * @param OutTransform The resulting transform from evaluated curve values
	 * @param CurveTable The Curve Table with rows: "LocationX", "LocationY", "LocationZ", "RotationPitch", "RotationYaw", "RotationRoll", "ScaleX", "ScaleY", "ScaleZ"
	 * @param TotalSecondsSinceStart The time in seconds to evaluate the curves
	 * @return Returns false if Curve Table is finished or cannot be evaluated */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool GetTransformFromCurveTable(FTransform& OutTransform, class UCurveTable* CurveTable, float TotalSecondsSinceStart);

	/** Applies transform from Curve Table to actor around a center point
	 * @param InActor The actor to animate
	 * @param CenterWorldTransform The pivot point for animation to prevent drift from actor's moving transform
	 * @param CurveTable The Curve Table with rows: "LocationX", "LocationY", "LocationZ", "RotationPitch", "RotationYaw", "RotationRoll", "ScaleX", "ScaleY", "ScaleZ"
	 * @param TotalSecondsSinceStart The time in seconds since animation started
	 * @return Returns false if Curve Table is finished or cannot be evaluated */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "CenterWorldTransform"))
	static bool ApplyTransformFromCurveTable(AActor* InActor, const FTransform& CenterWorldTransform, UCurveTable* CurveTable, float TotalSecondsSinceStart);

	/** Applies transform from Curve Table to scene component in own local space using actor transform as center.
	 * @param InComponent The scene component to animate
	 * @param CurveTable The Curve Table with transform curves
	 * @param TotalSecondsSinceStart The time in seconds since animation started
	 * @return Returns false if Curve Table is finished or cannot be evaluated */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static bool ApplyComponentTransformFromCurveTable(class USceneComponent* InComponent, UCurveTable* CurveTable, float TotalSecondsSinceStart);

	/*********************************************************************************************
	 * Level Helpers
	 ********************************************************************************************* */
public:
	/** Returns true if the specified level is opened in the current world. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsLevelOpened(const TSoftObjectPtr<UWorld>& Level);

	/** Opens the specified level as listen server: Level?listen.
	 * @param Level The level to open as listen server.
	 * @param bForceLoad If true, will open the level even if already in it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void OpenListenServerLevel(const TSoftObjectPtr<UWorld>& Level, bool bForceLoad = false);

	/*********************************************************************************************
	 * Ability System
	 ********************************************************************************************* */
public:
	/** Returns the tags owned by ASC that are descendants of Parent (Parent itself is excluded).
	 * Returns empty container if ASC is null or no tags match.
	 * @param ASC The Ability System Component to read owned tags from.
	 * @param Parent The parent tag used as the root of the match. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static struct FGameplayTagContainer GetFilteredGameplayTags(const class UAbilitySystemComponent* ASC, struct FGameplayTag Parent);
};
