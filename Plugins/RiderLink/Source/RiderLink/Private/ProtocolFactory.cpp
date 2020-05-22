#include "ProtocolFactory.h"

#include "rd_framework_cpp/scheduler/base/IScheduler.h"
#include "rd_framework_cpp/wire/SocketWire.h"

#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"

#include "HAL/PlatformFilemanager.h"
#include "Windows/WindowsPlatformMisc.h"

#include "Windows/PostWindowsApi.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "Runtime/Launch/Resources/Version.h"


TUniquePtr<rd::Protocol> ProtocolFactory::Create(rd::IScheduler * Scheduler, rd::Lifetime SocketLifetime)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 20
		TCHAR CAppDataLocalPath[4096];
		FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA"), CAppDataLocalPath, ARRAY_COUNT(CAppDataLocalPath));
        const FString FAppDataLocalPath = CAppDataLocalPath;
#else
        const FString FAppDataLocalPath = FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA"));
#endif

        const FString ProjectName = FApp::GetProjectName();
        const FString PortFullDirectoryPath = FPaths::Combine(*FAppDataLocalPath, TEXT("Jetbrains"), TEXT("Rider"),
                                                              TEXT("Unreal"), TEXT("Ports"));
        const FString PortFileFullPath = FPaths::Combine(PortFullDirectoryPath, *ProjectName);
        
        rd::minimum_level_to_log = rd::LogLevel::Error;
        auto wire = std::make_shared<rd::SocketWire::Server>(SocketLifetime, Scheduler, 0,
                                                             TCHAR_TO_UTF8(
                                                                 *FString::Printf(TEXT("UnrealEditorServer-%s"), *
                                                                     ProjectName)));
        auto protocol = MakeUnique<rd::Protocol>(rd::Identities::SERVER, Scheduler, wire, SocketLifetime);

        auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        if (PlatformFile.CreateDirectoryTree(*PortFullDirectoryPath)) {
            FFileHelper::SaveStringToFile(FString::FromInt(wire->port), *PortFileFullPath);
        }
    return protocol;
}
