// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Modules/ModuleInterface.h"

class NEWAI_API FNewAIModule final : public IModuleInterface
{
public:
	//~IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~End of IModuleInterface
};
