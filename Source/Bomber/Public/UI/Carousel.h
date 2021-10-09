// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "GameFramework/Actor.h"
#include "Carousel.generated.h"

/**
 * Data about spawned meshes for that floor.
 */
USTRUCT(BlueprintType)
struct FFloor
{
	GENERATED_BODY()

	/** Names and relative locations of spawned floors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	TMap<FString, FTransform> Meshes; //[M.AW]
};

/**
 * Spawn meshes of specified level type to preview and switch between them.
 */
UCLASS(Blueprintable, BlueprintType)
class ACarousel final : public AActor
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public
	 * --------------------------------------------------- */

	/** Default constructor. */
	ACarousel();

	/** Returns the chosen mesh component.
	 * @see ACarousel::CurrentMeshComponentInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UMeshComponent* GetCurrentMeshComponent() const { return CurrentMeshComponentInternal; }

	/** Returns the chosen mesh component. */
	template <typename T>
	FORCEINLINE T* GetCurrentMeshComponent() const { return Cast<T>(CurrentMeshComponentInternal); }

	/** Returns the current Level Actor Row of chosen mesh component.
	 * @see ACarousel::CurrentMeshRowInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class ULevelActorRow* GetCurrentMeshRow() const { return CurrentMeshRowInternal; }

	/** Returns the current Level Actor Row of chosen mesh component. */
	template <typename T>
	FORCEINLINE T* GetCurrentMeshRow() const { return Cast<T>(CurrentMeshRowInternal); }

protected:
	/* ---------------------------------------------------
	 *		Protected
	 * --------------------------------------------------- */

	/** Contains level actors data of spawned meshes for each floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Floors", ShowOnlyInnerProperties))
	TArray<FFloor> FloorsInternal; //[M.AW]

	/** The chosen mesh component. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current Mesh Component"))
	class UMeshComponent* CurrentMeshComponentInternal; //[G]

	/** The current Level Actor Row of chosen mesh component. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current Mesh Row"))
	class ULevelActorRow* CurrentMeshRowInternal; //[G]

	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

#if WITH_EDITOR	 // [Editor]Destroyed();
	/** Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending. */
	virtual void Destroyed() override;
#endif	// WITH_EDITOR [Editor]Destroyed();
};
