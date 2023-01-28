// Copyright (c) Yevhenii Selivanov.

#include "FunctionPickerEditorModule.h"
//---
#include "FunctionPickerType/FunctionPickerCustomization.h"

#define LOCTEXT_NAMESPACE "FFunctionPickerEditorModule"

// Called right after the module DLL has been loaded and the module object has been created
void FFunctionPickerEditorModule::StartupModule()
{
	FFunctionPickerCustomization::RegisterFunctionPickerCustomization();
}

// Called before the module is unloaded, right before the module object is destroyed
void FFunctionPickerEditorModule::ShutdownModule()
{
	FFunctionPickerCustomization::UnregisterFunctionPickerCustomization();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFunctionPickerEditorModule, FunctionPickerEditor)
