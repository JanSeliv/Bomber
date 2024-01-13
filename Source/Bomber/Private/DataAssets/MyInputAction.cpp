// Copyright (c) Yevhenii Selivanov

#include "DataAssets/MyInputAction.h"
//---
#if WITH_EDITOR
#include "Misc/DataValidation.h" // IsDataValid func
#endif // WITH_EDITOR
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyInputAction)

#if WITH_EDITOR
// Validates bound functions to this input action
EDataValidationResult UMyInputAction::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	const EDataValidationResult StaticContextResult = StaticContextInternal.IsDataValid(Context);
	Result = CombineDataValidationResults(Result, StaticContextResult);
	if (StaticContextResult == EDataValidationResult::Invalid)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("ERROR: 'Static Context' is invalid for next Input Action: '%s'"), *GetName())));
	}

	const EDataValidationResult FunctionToBindResult = FunctionToBindInternal.IsDataValid(Context);
	Result = CombineDataValidationResults(Result, FunctionToBindResult);
	if (FunctionToBindResult == EDataValidationResult::Invalid)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("ERROR: 'Function To Bind' is invalid for next Input Action: '%s'"), *GetName())));
	}

	return Result;
}
#endif // WITH_EDITOR
