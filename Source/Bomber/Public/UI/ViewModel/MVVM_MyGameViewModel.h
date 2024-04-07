// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Bomber.h"
#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "MVVM_MyGameViewModel.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Contains general data to be used only by widgets. 
 */
UCLASS(DisplayName = "My Game View Model")
class BOMBER_API UMVVM_MyGameViewModel : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

public:
	/** Setter and Getter widgets about the current game state. */
	UFUNCTION()
	void SetCurrentGameState(ECurrentGameState NewCurrentGameState);
	ECurrentGameState GetCurrentGameState() const { return CurrentGameState; }

protected:
	/** Represents the current game state.
	 * Is commonly used by 'UMyBlueprintFunctionLibrary::GetVisibilityByGameState' to show or hide own widget. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	ECurrentGameState CurrentGameState = ECurrentGameState::None;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/**  Is called when this View Model is constructed.
	* Is used for bindings to the changes in other systems in order to update own data. */
	virtual void OnViewModelConstruct_Implementation(const UUserWidget* UserWidget) override;

	/** Is called when this View Model is destructed. */
	virtual void OnViewModelDestruct_Implementation() override;
};