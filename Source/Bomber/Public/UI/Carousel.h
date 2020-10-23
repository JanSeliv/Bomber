// Copyright 2020 Yevhenii Selivanov

#pragma once

#include "GameFramework/Actor.h"
#include "Carousel.generated.h"


/**
 *
 */
USTRUCT(BlueprintType)
struct FFloor
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	TMap<FString, FTransform> Meshes; //[M.AW]
};

/**
 *
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

	/** Contains level actors data to spawn its meshes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Floors", ShowOnlyInnerProperties))
	TArray<FFloor> FloorsInternal; //[M.AW]

	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;
};
