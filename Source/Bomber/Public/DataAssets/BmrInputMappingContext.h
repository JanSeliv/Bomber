// Copyright (c) Yevhenii Selivanov

#pragma once

#include "InputMappingContext.h"

// UE
#include "GameplayTagContainer.h"

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
	const FGameplayTagContainer& GetActiveForStates() const { return ActiveForStatesTags; }

	/** Returns true if this input context has any game states assigned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool HasAnyActiveStates() const { return !ActiveForStatesTags.IsEmpty(); }

protected:
	/** If higher, then block the same consumed inputs other contexts with lower priorities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 ContextPriority = 0;

	/** Set the game states for which this input context should be active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, Categories = "GameState"))
	FGameplayTagContainer ActiveForStatesTags = FGameplayTagContainer::EmptyContainer;
};
