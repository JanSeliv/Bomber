// Copyright (c) Yevhenii Selivanov.

#include "MyEditorUtils.h"
//---
#include "FunctionPicker/FunctionPickerCustomization.h"
//---
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FMyEditorUtilsModule"

static const FName PropertyEditorModule = TEXT("PropertyEditor");

void FMyEditorUtilsModule::StartupModule()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
		PropertyEditorModule);

	// Allows to choose ufunction
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FFunctionPickerCustomization::PropertyClassName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFunctionPickerCustomization::MakeInstance)
	);

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FMyEditorUtilsModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(FFunctionPickerCustomization::PropertyClassName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMyEditorUtilsModule, MyEditorUtils)
