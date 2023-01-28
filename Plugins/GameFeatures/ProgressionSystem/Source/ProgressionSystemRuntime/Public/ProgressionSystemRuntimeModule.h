// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

/** Define Progression System log category. */
PROGRESSIONSYSTEMRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogProgressionSystem, Log, All);

class PROGRESSIONSYSTEMRUNTIME_API FProgressionSystemRuntimeModule : public IModuleInterface
{
public:
	//~IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~End of IModuleInterface
};
