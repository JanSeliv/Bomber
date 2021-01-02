// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "GameFramework/PlayerState.h"
//---
#include "Bomber.h"
//---
#include "MyPlayerState.generated.h"

/**
 *
 */
UCLASS()
class BOMBER_API AMyPlayerState final : public APlayerState
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public
	* --------------------------------------------------- */

	/** Default constructor. */
	AMyPlayerState();

	/** Returns result of the game for controlled player after ending the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE EEndGameState GetEndGameState() const { return EndGameStateInternal; }

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetChosenPlayerRaw(class UPlayerRow* ChosenPlayerRow);

	/** By which level type a skeletal mesh is chosen. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UPlayerRow* GetChosenPlayerRaw() const { return ChosenPlayerRowInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected
	* --------------------------------------------------- */

	/** Contains result of the game for controlled player after ending the game. */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "End Game State"))
	EEndGameState EndGameStateInternal = EEndGameState::None; //[N]

	/** a level type of chosen skeletal mesh. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Chosen Mesh"))
	UPlayerRow* ChosenPlayerRowInternal; //[G]

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when the game starts. Created widget. */
	virtual void BeginPlay() override;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Updated result of the game for controlled player after ending the game. Called when one of players is destroying. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (BlueprintProtected))
	void ServerUpdateEndState();
};
