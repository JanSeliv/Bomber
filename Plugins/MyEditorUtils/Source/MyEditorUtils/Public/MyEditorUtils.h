// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMyEditorUtilsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
