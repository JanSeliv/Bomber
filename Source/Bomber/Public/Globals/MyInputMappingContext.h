// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "InputMappingContext.h"
#include "Bomber.h"
//---
#include "MyInputMappingContext.generated.h"

/**
 * Contains specific for this project data and is intended to with UMyInputAction
 */
UCLASS(Blueprintable, Const, AutoExpandCategories=("C++"))
class UMyInputMappingContext final : public UInputMappingContext
{
	GENERATED_BODY()

public:
	/** Returns the game states for which this input context is active.
	 * @see ::ActiveForStatesInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetChosenGameStatesBitmask() const { return ActiveForStatesInternal; }

	/** Returns all input actions set in mappings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetInputActions(TArray<class UMyInputAction*>& OutInputActions) const;

	/** Returns mappings by specified input action.
	 * @param OutMappings Contains found mappings
	 * @param InputAction Action by that*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetMappingsByInputAction(TArray<FEnhancedActionKeyMapping>& OutMappings, const class UMyInputAction* InputAction) const;

protected:
	/** Set the game states for which this input context should be active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, Bitmask, BitmaskEnum = "ECurrentGameState", DisplayName = "Active For States", ShowOnlyInnerProperties))
	int32 ActiveForStatesInternal; //[D]
};
