// Copyright 2021 Yevhenii Selivanov.

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

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called after a successful login.  This is the first place it is safe to call replicated functions on the PlayerController. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Called when all players were connected. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++", meta = (BlueprintProtected))
	void OnSessionStarted() const;
};
