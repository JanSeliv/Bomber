// Copyright (c) Yevhenii Selivanov.

#include "MyEditorUtilsModule.h"
//---
#include "FunctionPicker/FunctionPickerCustomization.h"
#include "MyAssets/AssetTypeActions_MyUserWidget.h"
//---
#include "KismetCompiler.h"
#include "KismetCompilerModule.h"
#include "UMGEditorModule.h"
#include "Modules/ModuleManager.h"
#include "WidgetBlueprintCompiler.h"

#define LOCTEXT_NAMESPACE "FMyEditorUtilsModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMyEditorUtilsModule::StartupModule()
{
	RegisterPropertyCustomizations();
	RegisterMyUserWidgetBlueprint();
}

// Called before the module is unloaded, right before the module object is destroyed
void FMyEditorUtilsModule::ShutdownModule()
{
	UnregisterPropertyCustomizations();
	UnregisterMyUserWidgetBlueprint();
}

// Creates all customizations for custom properties
void FMyEditorUtilsModule::RegisterPropertyCustomizations()
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

// Removes all custom property customizations
void FMyEditorUtilsModule::UnregisterPropertyCustomizations()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(FFunctionPickerCustomization::PropertyClassName);
}

// Registers My User Widget Blueprint, so custom widget could be compiled
void FMyEditorUtilsModule::RegisterMyUserWidgetBlueprint()
{
	if (!FModuleManager::Get().IsModuleLoaded(UMGEditorModuleName)
		|| !FModuleManager::Get().IsModuleLoaded(KismetCompilerModuleName))
	{
		return;
	}

	FKismetCompilerContext::RegisterCompilerForBP(UMyUserWidgetBlueprint::StaticClass(), &UWidgetBlueprint::GetCompilerForWidgetBP);

	// Register widget blueprint compiler we do this no matter what.
	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>(UMGEditorModuleName);
	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>(KismetCompilerModuleName);
	KismetCompilerModule.GetCompilers().Add(UMGEditorModule.GetRegisteredCompiler());
}

// Unregisters My User Widget Blueprint
void FMyEditorUtilsModule::UnregisterMyUserWidgetBlueprint()
{
	if (!FModuleManager::Get().IsModuleLoaded(UMGEditorModuleName)
		|| !FModuleManager::Get().IsModuleLoaded(KismetCompilerModuleName))
	{
		return;
	}

	// Unregister widget blueprint compiler we do this no matter what.
	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>(UMGEditorModuleName);
	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>(KismetCompilerModuleName);
	KismetCompilerModule.GetCompilers().Remove(UMGEditorModule.GetRegisteredCompiler());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMyEditorUtilsModule, MyEditorUtils)
