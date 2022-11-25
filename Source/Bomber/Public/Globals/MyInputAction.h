// Copyright (c) Yevhenii Selivanov

#pragma once

#include "InputAction.h"
//---
#include "FunctionPicker.h"
#include "Bomber.h"
//---
#include "MyInputAction.generated.h"

/**
  * Is inherited data asset, has additional data to setup input action.
  */
UCLASS(Blueprintable, Const, AutoExpandCategories=("C++"))
class BOMBER_API UMyInputAction final : public UInputAction
{
	GENERATED_BODY()

public:
	/** Returns the input action name on UI.
	 * @see UMyInputAction::InputActionNameInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FText& GetInputActionName() const { return InputActionNameInternal; }

	/** Returns the chosen state when function has to be called.
	 * @see UMyInputAction::TriggerEventInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE ETriggerEvent GetTriggerEvent() const { return TriggerEventInternal; }

	/** Returns the data about static function object getter of a function to bind.
	 * @see UMyInputAction::StaticContextInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FFunctionPicker& GetStaticContext() const { return StaticContextInternal; }

	/** Returns the function data that is used to be called when input will be triggered.
	 * @see UMyInputAction::FunctionToBindInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FFunctionPicker& GetFunctionToBind() const { return FunctionToBindInternal; }

	/** Returns keys mapped to this action in the active input mapping contexts sorted by its priorities. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetKeys(TArray<FKey>& OutKeys) const;

	/** Returns the first mapped key to this action in most priority active input context. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FKey GetKey() const;

protected:
	/** The input action name on UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Input Action Name", ShowOnlyInnerProperties))
	FText InputActionNameInternal = TEXT_NONE; //[D]

	/** Choose for which state the bound function has to be called. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Trigger Event", ShowOnlyInnerProperties))
	ETriggerEvent TriggerEventInternal = ETriggerEvent::Triggered; //[D]

	/** Contains data about static function object getter of a function to bind. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Static Context", ShowOnlyInnerProperties, FunctionContextTemplate = "/Script/FunctionPicker.FunctionPickerTemplate::OnStaticContext__DelegateSignature"))
	FFunctionPicker StaticContextInternal = FFunctionPicker::Empty; //[D]

	/** Allows to set function that is used to be called when input will be triggered. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Function To Bind", ShowOnlyInnerProperties))
	FFunctionPicker FunctionToBindInternal = FFunctionPicker::Empty; //[D]
};
