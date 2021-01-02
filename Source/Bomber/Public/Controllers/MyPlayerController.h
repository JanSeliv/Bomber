// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Bomber.h"
//---
#include "MyPlayerController.generated.h"

/**
 * The player controller class
 */
UCLASS()
class BOMBER_API AMyPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	/** Sets default values for this controller's properties. */
	AMyPlayerController();

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++")
	bool CanHideMouse() const;

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "C++", meta = (DisplayName = "Set Game State"))
	void ServerSetGameState(ECurrentGameState NewGameState);

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetMouseCursor(bool bShouldShow);

protected:
	/** Allows the PlayerController to set up custom input bindings. */
	virtual void SetupInputComponent() override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/**
	* Locks or unlocks movement input, consecutive calls stack up and require the same amount of calls to undo, or can all be undone using ResetIgnoreMoveInput.
	* @param bShouldIgnore	If true, move input is ignored. If false, input is not ignored.
	*/
	virtual void SetIgnoreMoveInput(bool bShouldIgnore) override;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
