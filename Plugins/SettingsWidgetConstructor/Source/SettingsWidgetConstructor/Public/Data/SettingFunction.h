// Copyright (c) Yevhenii Selivanov

#pragma once

#include "SettingsWidgetConstructor/Private/FunctionPickerData/SWCFunctionPicker.h"
#include "SettingsWidgetConstructor/Private/FunctionPickerData/SWCFunctionPickerTemplate.h"
//---
#include "SettingFunction.generated.h"

/**
 * Is used to select setting function to be called
 */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingFunctionPicker : public FSWCFunctionPicker
{
	GENERATED_BODY()

	/** Empty setting function data. */
	static const FSettingFunctionPicker EmptySettingFunction;

	/** Default constructor. */
	FSettingFunctionPicker() = default;

	/** Custom constructor to set all members values. */
	FSettingFunctionPicker(UClass* InFunctionClass, FName InFunctionName)
		: FSWCFunctionPicker(InFunctionClass, InFunctionName) {}
};

/**
  * Delegates wrapper that are used as templates for FSettingFunctionPicker properties.
  * see UFunctionPickerTemplate
  */
UCLASS(Abstract, Const, Transient)
class SETTINGSWIDGETCONSTRUCTOR_API USettingFunctionTemplate : public USWCFunctionPickerTemplate
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterWidget, class USettingCustomWidget*, Param);

	DECLARE_DYNAMIC_DELEGATE_RetVal(class USettingCustomWidget*, FOnGetterWidget);
};
