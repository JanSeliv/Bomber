// Copyright 2020 Yevhenii Selivanov

#pragma once

#include "Camera/CameraActor.h"
//---
#include "MyCameraActor.generated.h"

/**
 * The main camera viewpoint of the game.
 */
UCLASS()
class BOMBER_API AMyCameraActor final : public ACameraActor
{
	GENERATED_BODY()

public:
	/** The max distance from the scene. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float MaxHeight;

	/** Sets default values for this actor's properties. */
	AMyCameraActor();

protected:
	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;
};
