// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Modules/ModuleInterface.h"

class MYUTILSNODES_API FMyUtilsNodesModule : public IModuleInterface
{
public:
	/** Called right after the module DLL has been loaded and the module object has been created */
	virtual void StartupModule() override;

	/** Called before the module is unloaded, right before the module object is destroyed */
	virtual void ShutdownModule() override;
};