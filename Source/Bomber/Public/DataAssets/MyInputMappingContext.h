// Copyright (c) Yevhenii Selivanov

#pragma once

#include "InputMappingContext.h"
//---
#include "MyInputMappingContext.generated.h"

/**
 * Contains specific for this project data and is intended to with UMyInputAction
 */
UCLASS(PerObjectConfig, Blueprintable, Const, AutoExpandCategories=("C++"))
class BOMBER_API UMyInputMappingContext final : public UInputMappingContext
{
	GENERATED_BODY()

public:
	/** Returns the priority of the context. If higher, then block the same consumed inputs other contexts with lower priorities.
	 * @see UMyInputMappingContext::ContextPriorityInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetContextPriority() const { return ContextPriorityInternal; }

	/** Returns the game states for which this input context is active.
	 * @see UMyInputMappingContext::ActiveForStatesInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetChosenGameStatesBitmask() const { return ActiveForStatesInternal; }

protected:
	/** If higher, then block the same consumed inputs other contexts with lower priorities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Context Priority"))
	int32 ContextPriorityInternal = 0;

	/** Set the game states for which this input context should be active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Active For States", Bitmask, BitmaskEnum = "/Script/Bomber.ECurrentGameState"))
	int32 ActiveForStatesInternal = 0;
};
