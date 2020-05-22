#include "RiderLoggingExtension.hpp"

#include "RiderLink.hpp"
#include "RdEditorProtocol/UE4Library/LogMessageInfo.h"
#include "RdEditorProtocol/UE4Library/UnrealLogEvent.h"

#include "Misc/DateTime.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "RiderLink"

DEFINE_LOG_CATEGORY(FLogRiderLoggingExtensionModule);

IMPLEMENT_MODULE(FRiderLoggingExtensionModule, RiderLoggingExtension);

void FRiderLoggingExtensionModule::StartupModule()
{
    UE_LOG(FLogRiderLoggingExtensionModule, Verbose, TEXT("STARTUP START"));

    static const auto START_TIME = FDateTime::UtcNow();
    static const auto GetTimeNow = [](double Time) -> rd::DateTime
    {
        return rd::DateTime(static_cast<std::time_t>(START_TIME.ToUnixTimestamp() +
            static_cast<int64>(Time)));
    };

    outputDevice.onSerializeMessage.BindLambda(
        [this](const TCHAR* msg, ELogVerbosity::Type Type,
               const class FName& Name, TOptional<double> Time)
        {
            if (Type > ELogVerbosity::All) return;

            FRiderLinkModule& RiderLinkModule = FModuleManager::GetModuleChecked<FRiderLinkModule>(
                FRiderLinkModule::GetModuleName());

            RiderLinkModule.RdConnection.Scheduler.queue([this, tail = FString(msg), Type,
                    Name = Name.GetPlainNameString(),
                    Time]() mutable
                {
                    rd::optional<rd::DateTime> DateTime;
                    if (Time)
                    {
                        DateTime = GetTimeNow(Time.GetValue());
                    }
                    Jetbrains::EditorPlugin::LogMessageInfo MessageInfo =
                        Jetbrains::EditorPlugin::LogMessageInfo(Type, Name, DateTime);

                    FRiderLinkModule& RiderLinkModule = FModuleManager::GetModuleChecked<
                        FRiderLinkModule>(
                        FRiderLinkModule::GetModuleName());

                    // [HACK]: fix https://github.com/JetBrains/UnrealLink/issues/17
                    // while we won't change BP hyperlink parsing
                    tail = tail.Left(4096);

                    FString toSend;
                    while (tail.Split("\n", &toSend, &tail))
                    {
                        toSend.TrimEndInline();
                        RiderLinkModule
                            .RdConnection.UnrealToBackendModel.get_unrealLog().fire(
                                Jetbrains::EditorPlugin::UnrealLogEvent{
                                    std::move(MessageInfo), std::move(toSend)
                                });
                    }
                    tail.TrimEndInline();
                    RiderLinkModule.RdConnection.UnrealToBackendModel.get_unrealLog().fire(
                        Jetbrains::EditorPlugin::UnrealLogEvent{
                            MessageInfo, std::move(tail)
                        });
                });
        });

    UE_LOG(FLogRiderLoggingExtensionModule, Verbose, TEXT("STARTUP FINISH"));
}

void FRiderLoggingExtensionModule::ShutdownModule()
{
    UE_LOG(FLogRiderLoggingExtensionModule, Verbose, TEXT("SHUTDOWN START"));
    outputDevice.onSerializeMessage.Unbind();
    UE_LOG(FLogRiderLoggingExtensionModule, Verbose, TEXT("SHUTDOWN FINISH"));
}
