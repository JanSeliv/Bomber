#pragma once

#if PLATFORM_WINDOWS
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"

#include "rd_framework_cpp/protocol/Protocol.h"

#include "Windows/PostWindowsApi.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "Templates/UniquePtr.h"

class ProtocolFactory {
public:
    static TUniquePtr<rd::Protocol> Create(rd::IScheduler * Scheduler, rd::Lifetime SocketLifetime);
};
