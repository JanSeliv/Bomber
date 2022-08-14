// Copyright (c) Yevhenii Selivanov.

#include "MyEditorUtilsModule.h"
//---
#include "FunctionPicker/FunctionPickerCustomization.h"
//---
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FMyEditorUtilsModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMyEditorUtilsModule::StartupModule()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Allows to choose ufunction
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FFunctionPickerCustomization::PropertyClassName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFunctionPickerCustomization::MakeInstance)
	);

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Called before the module is unloaded, right before the module object is destroyed
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
