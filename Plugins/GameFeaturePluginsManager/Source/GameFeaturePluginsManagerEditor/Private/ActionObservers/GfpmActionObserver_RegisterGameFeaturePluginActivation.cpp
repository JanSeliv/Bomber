// Copyright (c) Yevhenii Selivanov

#include "ActionObservers/GfpmActionObserver_RegisterGameFeaturePluginActivation.h"

// GFPM
#include "GameFeatureActions/GfpmAction_RegisterGameFeaturePluginActivation.h"
#include "Subsystems/GfpmLoaderSubsystem.h"

// UE
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFeatureData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmActionObserver_RegisterGameFeaturePluginActivation)

// Identifies the action type this observer handles
TSubclassOf<UGameFeatureAction> UGfpmActionObserver_RegisterGameFeaturePluginActivation::GetObservedActionClass() const
{
	return UGfpmAction_RegisterGameFeaturePluginActivation::StaticClass();
}

// When owning plugin is registered
void UGfpmActionObserver_RegisterGameFeaturePluginActivation::OnGameFeatureRegistering()
{
	MirrorAcrossWorlds(/*bRegister=*/true);
}

// When owning plugin is unregistering
void UGfpmActionObserver_RegisterGameFeaturePluginActivation::OnGameFeatureUnregistering()
{
	MirrorAcrossWorlds(/*bRegister=*/false);
}

// Mirrors observed action's activation entry into every live world's tag-driven loader, or drops it when bRegister is false
void UGfpmActionObserver_RegisterGameFeaturePluginActivation::MirrorAcrossWorlds(bool bRegister) const
{
	const UGfpmAction_RegisterGameFeaturePluginActivation* Action = Cast<UGfpmAction_RegisterGameFeaturePluginActivation>(ObservedAction.Get());
	if (!ensureMsgf(Action && GEngine, TEXT("ASSERT: [%i] %hs:\n'Action && GEngine' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const UGameFeatureData* GameFeatureData = Action->GetTypedOuter<UGameFeatureData>();
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		const UWorld* World = WorldContext.World();
		UGfpmLoaderSubsystem* Loader = World ? World->GetSubsystem<UGfpmLoaderSubsystem>() : nullptr;
		if (!Loader)
		{
			// Stale world context or world without the loader, skip
			continue;
		}

		if (bRegister)
		{
			Loader->RegisterGameFeaturePluginActivation(GameFeatureData, Action->ActivationTags);
		}
		else
		{
			Loader->UnregisterGameFeaturePluginActivation(GameFeatureData);
		}
	}
}
