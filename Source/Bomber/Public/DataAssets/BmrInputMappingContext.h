// Copyright (c) Yevhenii Selivanov

#pragma once

#include "InputMappingContext.h"

#include "BmrInputMappingContext.generated.h"

/**
 * Contains specific for this project data and is intended to with UBmrInputAction
 */
UCLASS(PerObjectConfig, Blueprintable, Const, AutoExpandCategories = ("[Bomber]"))
class BOMBER_API UBmrInputMappingContext final : public UInputMappingContext
{
	GENERATED_BODY()

public:
	/** Returns the priority of the context. If higher, then block the same consumed inputs other contexts with lower priorities. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetContextPriority() const { return ContextPriority; }

	/** Returns the game states for which this input context is active. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetChosenGameStatesBitmask() const { return ActiveForStates; }

protected:
	/** If higher, then block the same consumed inputs other contexts with lower priorities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 ContextPriority = 0;

	/** Set the game states for which this input context should be active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, Bitmask, BitmaskEnum = "/Script/Bomber.EBmrCurrentGameState"))
	int32 ActiveForStates = 0;
};
