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
	/** Returns the priority of the context. If higher, then block the same consumed inputs other contexts with lower priorities.
	 * @see UMyInputMappingContext::ContextPriorityInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetContextPriority() const { return ContextPriorityInternal; }

	/** Returns the game states for which this input context is active.
	 * @see UMyInputMappingContext::ActiveForStatesInternal */
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
	/** If higher, then block the same consumed inputs other contexts with lower priorities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Context Priority"))
	int32 ContextPriorityInternal; //[D]

	/** Set the game states for which this input context should be active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Active For States", Bitmask, BitmaskEnum = "ECurrentGameState"))
	int32 ActiveForStatesInternal; //[D]
};
