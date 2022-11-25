// Copyright (c) Yevhenii Selivanov.

#include "FunctionPickerEditorModule.h"
//---
#include "FunctionPickerCustomization.h"
#include "MyEditorUtilsModule.h"

#define LOCTEXT_NAMESPACE "FFunctionPickerEditorModule"

// Called right after the module DLL has been loaded and the module object has been created
void FFunctionPickerEditorModule::StartupModule()
{
	RegisterFunctionPickerCustomization();
}

// Called before the module is unloaded, right before the module object is destroyed
void FFunctionPickerEditorModule::ShutdownModule()
{
	UnregisterPropertyCustomizations();
}

// Created customization for the Function Picker
void FFunctionPickerEditorModule::RegisterFunctionPickerCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(FMyEditorUtilsModule::PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(FMyEditorUtilsModule::PropertyEditorModule);

	// Allows to choose ufunction
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FFunctionPickerCustomization::PropertyClassName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFunctionPickerCustomization::MakeInstance)
	);

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization customization for the Function Picker
void FFunctionPickerEditorModule::UnregisterPropertyCustomizations()
{
	if (!FModuleManager::Get().IsModuleLoaded(FMyEditorUtilsModule::PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(FMyEditorUtilsModule::PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(FFunctionPickerCustomization::PropertyClassName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFunctionPickerEditorModule, FunctionPickerEditor)
