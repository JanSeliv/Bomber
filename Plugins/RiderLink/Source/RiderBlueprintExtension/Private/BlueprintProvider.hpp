#pragma once

#include "AssetData.h"
#include "MessageEndpoint.h"

DECLARE_DELEGATE_OneParam(FOnBlueprintAdded, UBlueprint *);

class BluePrintProvider {
    static void AddBlueprint(UBlueprint* Blueprint);
public:
    static FOnBlueprintAdded OnBlueprintAdded;

    static void AddAsset(FAssetData const& AssetData);

    static bool IsBlueprint(FString const& pathName);

    static void OpenBlueprint(FString const& path, TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> const& messageEndpoint);
};
