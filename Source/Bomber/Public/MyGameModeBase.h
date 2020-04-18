// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameModeBase.h"
//---
#include "MyGameModeBase.generated.h"

/**
 * The custom game mode class
 */
UCLASS()
class BOMBER_API AMyGameModeBase final : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AMyGameModeBase();

	/** The class of the camera actor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TSubclassOf<AActor> CameraActorClass;  //[B]

	/** The class of the camera actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float Timer = 120.0F;

protected:
	/** Called when the game starts or when spawned */
	void BeginPlay() override;
};
