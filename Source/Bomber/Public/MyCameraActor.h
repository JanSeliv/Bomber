// Copyright 2020 Yevhenii Selivanov

#pragma once

#include "Camera/CameraActor.h"
#include "Bomber.h"
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

	/** Returns the time taken to blend. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetBlendTime() const { return BlendTimeInternal; }

protected:
	/** Time taken to blend.
	* @TODO Replace to the Camera Data Asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Blend Time"))
	float BlendTimeInternal = 1.F; //[N]

	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
    void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
