#include "RiderBlueprintExtension.hpp"

#if PLATFORM_WINDOWS
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/AllowWindowsPlatformTypes.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/PreWindowsApi.h"

#include "RdEditorProtocol/RdEditorModel/RdEditorModel.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/PostWindowsApi.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "BlueprintProvider.hpp"
#include "RiderLink.hpp"

#include "AssetRegistryModule.h"
#include "HAL/PlatformProcess.h"
#include "MessageEndpoint.h"
#include "MessageEndpointBuilder.h"
#include "Engine/Blueprint.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "RiderLink"

DEFINE_LOG_CATEGORY(FLogRiderBlueprintExtensionModule);

IMPLEMENT_MODULE(FRiderBlueprintExtensionModule, RiderBlueprintExtension);

static void AllowSetForeGroundForEditor(Jetbrains::EditorPlugin::RdEditorModel const & unrealToBackendModel) {
    static const int32 CurrentProcessId = FPlatformProcess::GetCurrentProcessId();
    try {
        const rd::WiredRdTask<bool> Task = unrealToBackendModel.get_allowSetForegroundWindow().sync(CurrentProcessId);
        if (Task.is_faulted()) {
            UE_LOG(FLogRiderBlueprintExtensionModule, Error, TEXT("AllowSetForeGroundForEditor failed: %hs "), rd::to_string(Task.value_or_throw()).c_str());
        }
        else if (Task.is_succeeded()) {
            if (!(Task.value_or_throw().unwrap())) {
                UE_LOG(FLogRiderBlueprintExtensionModule, Error, TEXT("AllowSetForeGroundForEditor failed: %hs "), rd::to_string(Task.value_or_throw()).c_str());
            }
        }
    }
    catch (std::exception const &e) {
        UE_LOG(FLogRiderBlueprintExtensionModule, Error, TEXT("AllowSetForeGroundForEditor failed: %hs "), rd::to_string(e).c_str());
    }
}

void FRiderBlueprintExtensionModule::StartupModule()
{
    UE_LOG(FLogRiderBlueprintExtensionModule, Verbose, TEXT("STARTUP START"));

    FRiderLinkModule& RiderLinkModule = FModuleManager::GetModuleChecked<
                        FRiderLinkModule>(
                        FRiderLinkModule::GetModuleName());

    const FAssetRegistryModule* AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>
        (AssetRegistryConstants::ModuleName);

    MessageEndpoint = FMessageEndpoint::Builder(FName("FAssetEditorManager")).Build();

    AssetRegistryModule->Get().OnAssetAdded().AddLambda([](const FAssetData& AssetData) {
        // TODO: Fix loading uasset's on 4.23-
        // BluePrintProvider::AddAsset(AssetData);
    });
    Jetbrains::EditorPlugin::RdEditorModel& UnrealToBackendModel = FRiderLinkModule::Get().RdConnection.UnrealToBackendModel;
    const rd::Lifetime NestedLifetime = RiderLinkModule.CreateNestedLifetime();
    UnrealToBackendModel.get_openBlueprint().advise( NestedLifetime,
[this, &UnrealToBackendModel](Jetbrains::EditorPlugin::BlueprintReference const& s) {
        try {
            AllowSetForeGroundForEditor(UnrealToBackendModel);

            auto Window = FGlobalTabmanager::Get()->GetRootWindow();
            if (Window->IsWindowMinimized()) {
                Window->Restore();
            } else {
                Window->HACK_ForceToFront();
            }
            BluePrintProvider::OpenBlueprint(s.get_pathName(), MessageEndpoint);
        } catch (std::exception const& e) {
            std::cerr << rd::to_string(e);
        }
    });

    UnrealToBackendModel.get_isBlueprintPathName().set([](FString const& pathName) -> bool {
        return BluePrintProvider::IsBlueprint(pathName);
    });


    BluePrintProvider::OnBlueprintAdded.BindLambda([this](UBlueprint* Blueprint) {
        FRiderLinkModule::Get().RdConnection.Scheduler.queue([this, Blueprint] {
            FRiderLinkModule::Get().RdConnection.UnrealToBackendModel.get_onBlueprintAdded().fire(
                Jetbrains::EditorPlugin::UClass(Blueprint->GetPathName()));
        });
    });
    
    UE_LOG(FLogRiderBlueprintExtensionModule, Verbose, TEXT("STARTUP FINISH"));
}

void FRiderBlueprintExtensionModule::ShutdownModule()
{
    UE_LOG(FLogRiderBlueprintExtensionModule, Verbose, TEXT("SHUTDOWN START"));

    UE_LOG(FLogRiderBlueprintExtensionModule, Verbose, TEXT("SHUTDOWN FINISH"));
}
