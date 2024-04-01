// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Modules/ModuleInterface.h"
//---
#include "UObject/NameTypes.h"

class MYEDITORUTILS_API FMyEditorUtilsModule final : public IModuleInterface
{
public:
	/** Is used to load and unload the Property Editor Module. */
	inline static const FName PropertyEditorModule = TEXT("PropertyEditor");

	/** Are used to register and unregister custom widget blueprint. */
	inline static const FName UMGEditorModuleName = TEXT("UMGEditor");
	inline static const FName KismetCompilerModuleName = TEXT("KismetCompiler");

	/**
	 * Called right after the module DLL has been loaded and the module object has been created.
	 * Load dependent modules here, and they will be guaranteed to be available during ShutdownModule.
	 */
	virtual void StartupModule() override;

	/**
	* Called before the module is unloaded, right before the module object is destroyed.
	* During normal shutdown, this is called in reverse order that modules finish StartupModule().
	* This means that, as long as a module references dependent modules in it's StartupModule(), it
	* can safely reference those dependencies in ShutdownModule() as well.
	*/
	virtual void ShutdownModule() override;

protected:
	/** Registers My User Widget Blueprint, so custom widget could be compiled. */
	void RegisterMyUserWidgetBlueprint();

	/** Unregisters My User Widget Blueprint. */
	void UnregisterMyUserWidgetBlueprint();
};
