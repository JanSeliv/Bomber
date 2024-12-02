// Copyright (c) Yevhenii Selivanov

#include "FTGEditorUtils.h"
//---
#include "FootTrailsGeneratorRuntimeModule.h"
#include "FTGComponent.h"
//---
#include "GameFeatureAction_AddComponents.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(FTGEditorUtils)

// Tries to obtain the blueprint class of the Foot Trails Generator component from MGF data asset, where it's expected to be set
TSubclassOf<class UFTGComponent> UFTGEditorUtils::GetFootTrailsComponentClass()
{
	const UGameFeaturesSubsystem& MGFSubsystem = UGameFeaturesSubsystem::Get();

	FString PluginURL;
	if (!MGFSubsystem.GetPluginURLByName(FFootTrailsGeneratorRuntimeModule::FTGPluginName.ToString(), /*Out*/ PluginURL))
	{
		// Plugin is disabled
		return nullptr;
	}

	const UGameFeatureData* MGFData = UGameFeaturesSubsystem::Get().GetGameFeatureDataForRegisteredPluginByURL(PluginURL);
	if (!ensureMsgf(MGFData, TEXT("ASSERT: [%i] %hs:\n'MGFData' was not found!"), __LINE__, __FUNCTION__))
	{
		return nullptr;
	}

	const TArray<UGameFeatureAction*>& AllActions = MGFData->GetActions();
	for (const UGameFeatureAction* ActionIt : AllActions)
	{
		const UGameFeatureAction_AddComponents* ComponentAction = Cast<UGameFeatureAction_AddComponents>(ActionIt);
		if (!ComponentAction)
		{
			continue;
		}

		for (const FGameFeatureComponentEntry& ComponentListIt : ComponentAction->ComponentList)
		{
			const TSubclassOf<UActorComponent> ComponentClass = !ComponentListIt.ComponentClass.IsNull() ? ComponentListIt.ComponentClass.LoadSynchronous() : nullptr;
			if (ComponentClass && ComponentClass->IsChildOf<UFTGComponent>())
			{
				return ComponentClass.Get();
			}
		}
	}

	return nullptr;
}
