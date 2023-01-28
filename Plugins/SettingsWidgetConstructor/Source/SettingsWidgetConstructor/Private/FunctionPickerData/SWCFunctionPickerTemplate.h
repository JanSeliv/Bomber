// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UObject/Object.h"
//---
#include "SWCFunctionPickerTemplate.generated.h"

/**
  * Delegates wrapper that are used as templates for FFunctionPicker properties.
  * Has to have reflection to allow find its members by FFunctionPickerCustomization:
  * UFunctionPickerTemplate::StaticClass()->FindFunctionByName("OnStaticContext__DelegateSignature");
  * DECLARE_DYNAMIC_DELEGATE can't be declared under USTRUCT
  */
UCLASS(Abstract, Const, Transient)
class USWCFunctionPickerTemplate : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(UObject*, FOnStaticContext);

	DECLARE_DYNAMIC_DELEGATE(FOnButtonPressed);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterInt, int32, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterFloat, double, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterBool, bool, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterText, FText, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetMembers, const TArray<FText>&, NewMembers);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterName, FName, Param);

	DECLARE_DYNAMIC_DELEGATE_RetVal(int32, FOnGetterInt);

	DECLARE_DYNAMIC_DELEGATE_RetVal(double, FOnGetterFloat);

	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FOnGetterBool);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetterText, FText&, OutParam);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetMembers, TArray<FText>&, OutParam);

	DECLARE_DYNAMIC_DELEGATE_RetVal(FName, FOnGetterName);
};