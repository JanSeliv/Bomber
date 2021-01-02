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

protected:
	/* ---------------------------------------------------
	 *		Protected
	 * --------------------------------------------------- */

	/** Contains level actors data of spawned meshes for each floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Floors", ShowOnlyInnerProperties))
	TArray<FFloor> FloorsInternal; //[M.AW]

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
