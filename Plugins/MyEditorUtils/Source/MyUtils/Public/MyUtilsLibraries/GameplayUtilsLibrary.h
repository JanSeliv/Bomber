// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
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
};
