// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrInputAction.h"

// UE
#if WITH_EDITOR
#include "Misc/DataValidation.h" // IsDataValid func
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrInputAction)

// Contains no binding data
const FBmrInputActionBinding FBmrInputActionBinding::Empty = FBmrInputActionBinding();

#if WITH_EDITOR
// Validates bound functions to this input action
EDataValidationResult UBmrInputAction::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	for (int32 Index = 0; Index < InputActionBindings.Num(); ++Index)
	{
		const FBmrInputActionBinding& CurrentBinding = InputActionBindings[Index];

		const EDataValidationResult StaticContextResult = CurrentBinding.StaticContext.IsDataValid(Context);
		Result = CombineDataValidationResults(Result, StaticContextResult);
		if (StaticContextResult == EDataValidationResult::Invalid)
		{
			Result = EDataValidationResult::Invalid;
			static const FString StaticContextTemplated = TEXT("ERROR: 'Static Context' is invalid for Input Action Binding [{0}] in Input Action: '{1}'");
			const FString StaticContextFormatted = FString::Format(*StaticContextTemplated, {Index, *GetName()});
			Context.AddError(FText::FromString(StaticContextFormatted));
		}

		const EDataValidationResult FunctionToBindResult = CurrentBinding.FunctionToBind.IsDataValid(Context);
		Result = CombineDataValidationResults(Result, FunctionToBindResult);
		if (FunctionToBindResult == EDataValidationResult::Invalid)
		{
			Result = EDataValidationResult::Invalid;
			static const FString FunctionToBindTemplated = TEXT("ERROR: 'Function To Bind' is invalid for Input Action Binding [{0}] in Input Action: '{1}'");
			const FString FunctionToBindFormatted = FString::Format(*FunctionToBindTemplated, {Index, *GetName()});
			Context.AddError(FText::FromString(FunctionToBindFormatted));
		}
	}

	return Result;
}
#endif // WITH_EDITOR