// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(FLogRiderGameControlExtensionModule, Log, All);

class FRiderGameControlExtensionModule : public IModuleInterface
{
public:
    FRiderGameControlExtensionModule() = default;
    ~FRiderGameControlExtensionModule() = default;

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual bool SupportsDynamicReloading() override { return true; };
    
private:
    bool PlayFromUnreal = false;
    bool PlayFromRider = false;
};
