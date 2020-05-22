// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "RiderLink.hpp"

#include "Modules/ModuleManager.h"

#if PLATFORM_WINDOWS
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"

#include "HAL/Platform.h"

#include "Windows/PostWindowsApi.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#define LOCTEXT_NAMESPACE "RiderLink"

DEFINE_LOG_CATEGORY(FLogRiderLinkModule);

IMPLEMENT_MODULE(FRiderLinkModule, RiderLink);

void FRiderLinkModule::ShutdownModule()
{
  UE_LOG(FLogRiderLinkModule, Verbose, TEXT("SHUTDOWN START"));
  RdConnection.Shutdown();
  UE_LOG(FLogRiderLinkModule, Verbose, TEXT("SHUTDOWN FINISH"));
}

void FRiderLinkModule::StartupModule()
{
  UE_LOG(FLogRiderLinkModule, Verbose, TEXT("STARTUP START"));
  RdConnection.Init();
  UE_LOG(FLogRiderLinkModule, Verbose, TEXT("STARTUP FINISH"));
}

bool FRiderLinkModule::SupportsDynamicReloading() { return true; }

#undef LOCTEXT_NAMESPACE
