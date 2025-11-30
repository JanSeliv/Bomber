// Copyright (c) Yevhenii Selivanov

#pragma once

// Bomber
#include "FunctionPickerData/FunctionPicker.h"
#include "InputTriggers.h"

#include "BmrInputActionBinding.generated.h"

/**
 * Represents a binding information to which function will be called when input is triggered.
 * Is setup from IA_ assets.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrInputActionBinding
{
	GENERATED_BODY()

	/** Contains no binding data. */
	static const FBmrInputActionBinding Empty;

	/** Choose for which state the bound function has to be called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (ShowOnlyInnerProperties))
	ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;

	/** Contains data about static function object getter of a function to bind. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (ShowOnlyInnerProperties, FunctionContextTemplate = "/Script/FunctionPicker.FunctionPickerTemplate::OnGetterObject__DelegateSignature"))
	FFunctionPicker StaticContext = FFunctionPicker::Empty;

	/** Allows to set function that is used to be called when input will be triggered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (ShowOnlyInnerProperties))
	FFunctionPicker FunctionToBind = FFunctionPicker::Empty;
};