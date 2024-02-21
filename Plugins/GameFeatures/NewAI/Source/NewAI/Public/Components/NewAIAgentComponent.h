// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "Bomber.h"
//---
#include "NewAIAgentComponent.generated.h"

class AAIController;

/**
 * Is activated for each AI Controller (AI agents known as ‘bots’) on starting the game.
 */
UCLASS(Blueprintable, DisplayName = "NewAI Agent Component", ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWAI_API UNewAIAgentComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UNewAIAgentComponent();

	/** Returns AI Controller of this component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	AAIController* GetAIController() const;
	AAIController& GetAIControllerChecked() const;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:

	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Starts or stops the behavior tree. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void HandleBehaviorTree();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
public:
	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Called when new difficulty level is set. */
	UFUNCTION(BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnNewAIDifficultyChanged(int32 NewDifficultyLevel);
};
