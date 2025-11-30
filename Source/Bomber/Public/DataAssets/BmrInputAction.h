// Copyright (c) Yevhenii Selivanov

#pragma once

#include "InputAction.h"

// Bomber
#include "Structures/BmrInputActionBinding.h"

#include "BmrInputAction.generated.h"

/**
 * Is inherited data asset, has additional data to setup input action.
 */
UCLASS(Blueprintable, Const, AutoExpandCategories = ("[Bomber]"))
class BOMBER_API UBmrInputAction final : public UInputAction
{
	GENERATED_BODY()

public:
	/** Returns the number of input action bindings configured for this action. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetInputActionBindingsNum() const { return InputActionBindings.Num(); }

	/** Returns the input action binding by index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FBmrInputActionBinding& GetInputActionBinding(int32 Index) const { return InputActionBindings.IsValidIndex(Index) ? InputActionBindings[Index] : FBmrInputActionBinding::Empty; }

#if WITH_EDITOR
	/** Validates bound functions to this input action. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

protected:
	/** Contains all input action binding configurations for this action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<FBmrInputActionBinding> InputActionBindings;
};