// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "Bomber.h"
//---
#include "NewAIAgentComponent.generated.h"

/**
 * Is activated for each AI Controller (AI agents known as ‘bots’) on starting the game.
 */
UCLASS(Blueprintable, DisplayName = "NewAI Agent Component", ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWAIRUNTIME_API UNewAIAgentComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UNewAIAgentComponent();

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The AI Controller that owns this component. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "My AI Controller"))
	TObjectPtr<class AAIController> AIControllerInternal = nullptr;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
