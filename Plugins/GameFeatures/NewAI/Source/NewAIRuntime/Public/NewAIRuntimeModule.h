// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Modules/ModuleInterface.h"

/** Define Bomber log category. */
NEWAIRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogNewAI, Log, All);

class NEWAIRUNTIME_API FNewAIRuntimeModule final : public IModuleInterface
{
public:
	//~IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~End of IModuleInterface
};
