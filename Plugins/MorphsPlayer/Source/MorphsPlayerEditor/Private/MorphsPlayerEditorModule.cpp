// Copyright (c) Yevhenii Selivanov.

#include "MorphsPlayerEditorModule.h"
//---
#include "MorphDataCustomization.h"

#define LOCTEXT_NAMESPACE "FMorphsPlayerEditorModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMorphsPlayerEditorModule::StartupModule()
{
	FMorphDataCustomization::RegisterMorphDataCustomization();
}

// Called before the module is unloaded, right before the module object is destroyed
void FMorphsPlayerEditorModule::ShutdownModule()
{
	FMorphDataCustomization::UnregisterMorphDataCustomization();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FMorphsPlayerEditorModule, MorphsPlayerEditor)