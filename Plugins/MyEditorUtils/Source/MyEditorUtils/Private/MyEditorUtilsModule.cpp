// Copyright (c) Yevhenii Selivanov.

#include "MyEditorUtilsModule.h"
//---
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
	RegisterMyUserWidgetBlueprint();
}

// Called before the module is unloaded, right before the module object is destroyed
void FMyEditorUtilsModule::ShutdownModule()
{
	UnregisterMyUserWidgetBlueprint();
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
