// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPlayAreaSurrounderRuntimeModule : public IModuleInterface
{
public:
	//~IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~End of IModuleInterface
};
