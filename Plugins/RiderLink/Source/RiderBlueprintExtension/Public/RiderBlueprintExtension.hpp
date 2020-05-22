// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MessageEndpoint.h"
#include "Templates/SharedPointer.h"
#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(FLogRiderBlueprintExtensionModule, Log, All);

class FRiderBlueprintExtensionModule : public IModuleInterface
{
public:
    FRiderBlueprintExtensionModule() = default;
    ~FRiderBlueprintExtensionModule() = default;

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual bool SupportsDynamicReloading() override { return true; };
private:
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> MessageEndpoint;
};
