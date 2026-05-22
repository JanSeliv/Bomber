// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GfpmActionObserversEditorSubsystem.h"

// GFPM
#include "ActionObservers/GfpmActionObserver_Base.h"
#include "GfpmUtils.h"

// UE
#include "GameFeatureAction.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "Subsystems/SubsystemCollection.h"
#include "UObject/UObjectIterator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmActionObserversEditorSubsystem)

// When subsystem initializes
void UGfpmActionObserversEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Engine subsystem init order is unspecified, force the game features subsystem up first since this observer registers with it right below
	Collection.InitializeDependency<UGameFeaturesSubsystem>();

	UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();
	GameFeaturesSubsystem.AddObserver(this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::FutureOnly);

	// Seed: replay lifecycle for plugins already registered or active when this subsystem comes online, AddObserver does not replay past transitions
	GameFeaturesSubsystem.ForEachGameFeature([this, &GameFeaturesSubsystem](FGameFeatureInfo&& Info)
	{
		const UGameFeatureData* Data = GameFeaturesSubsystem.GetGameFeatureDataForRegisteredPluginByURL(Info.URL);
		if (!Data)
		{
			// Plugin not registered yet, its actions do not exist
			return;
		}

		OnGameFeatureRegistering(Data, Info.Name, Info.URL);

		if (UGfpmUtils::IsGameFeaturePluginActive(FName(Info.Name)))
		{
			OnGameFeatureActivating(Data, Info.URL);
			OnGameFeatureActivated(Data, Info.URL);
		}
	});
}

// When subsystem is destroyed
void UGfpmActionObserversEditorSubsystem::Deinitialize()
{
	UGameFeaturesSubsystem::Get().RemoveObserver(this);

	Observers.Reset();

	Super::Deinitialize();
}

// When a game feature plugin is registered
void UGfpmActionObserversEditorSubsystem::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	if (!GameFeatureData)
	{
		return;
	}

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		const bool bConcreteObserver = ClassIt->IsChildOf(UGfpmActionObserver_Base::StaticClass())
		                               && !ClassIt->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists);
		if (!bConcreteObserver)
		{
			continue;
		}

		const UGfpmActionObserver_Base* ObserverCDO = GetDefault<UGfpmActionObserver_Base>(*ClassIt);
		if (ObserverCDO && !ObserverCDO->ShouldCreateObserver())
		{
			// Observer is not designed for this configuration, e.g. editor-only which skips currently running -game
			continue;
		}

		const TSubclassOf<UGameFeatureAction> ObservedClass = ObserverCDO ? ObserverCDO->GetObservedActionClass() : nullptr;
		if (!ensureMsgf(ObservedClass, TEXT("ASSERT: [%i] %hs:\nObserver class '%s' declares no observed action class!"), __LINE__, __FUNCTION__, *ClassIt->GetName()))
		{
			// Observer declares no action type, nothing to match against
			continue;
		}

		for (UGameFeatureAction* Action : GameFeatureData->GetActions())
		{
			if (!Action || !Action->IsA(ObservedClass))
			{
				continue;
			}

			if (UGfpmActionObserver_Base* Observer = FindOrCreateObserver(PluginURL, *ClassIt))
			{
				Observer->SetObservedAction(Action, PluginURL);
				Observer->OnGameFeatureRegistering();
			}

			// One observer per plugin per observer class, ignore extra actions of same type
			break;
		}
	}
}

// When a game feature plugin begins transition into Active state
void UGfpmActionObserversEditorSubsystem::OnGameFeatureActivating(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	for (UGfpmActionObserver_Base* Observer : Observers)
	{
		if (Observer && Observer->GetPluginURL() == PluginURL)
		{
			Observer->OnGameFeatureActivating();
		}
	}
}

// When a game feature plugin finished transition into Active state
void UGfpmActionObserversEditorSubsystem::OnGameFeatureActivated(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	for (UGfpmActionObserver_Base* Observer : Observers)
	{
		if (Observer && Observer->GetPluginURL() == PluginURL)
		{
			Observer->OnGameFeatureActivated();
		}
	}
}

// When a game feature plugin transitions out of Active state
void UGfpmActionObserversEditorSubsystem::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL)
{
	for (UGfpmActionObserver_Base* Observer : Observers)
	{
		if (Observer && Observer->GetPluginURL() == PluginURL)
		{
			Observer->OnGameFeatureDeactivating();
		}
	}
}

// When a game feature plugin is unregistering
void UGfpmActionObserversEditorSubsystem::OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	for (UGfpmActionObserver_Base* Observer : Observers)
	{
		if (Observer && Observer->GetPluginURL() == PluginURL)
		{
			Observer->OnGameFeatureUnregistering();
		}
	}
}

// Returns observer of given class bound to given plugin URL, creating and storing it on first request
UGfpmActionObserver_Base* UGfpmActionObserversEditorSubsystem::FindOrCreateObserver(const FString& PluginURL, TSubclassOf<UGfpmActionObserver_Base> ObserverClass)
{
	if (!ObserverClass)
	{
		return nullptr;
	}

	for (UGfpmActionObserver_Base* Observer : Observers)
	{
		if (Observer
		    && Observer->GetClass() == ObserverClass
		    && Observer->GetPluginURL() == PluginURL)
		{
			return Observer;
		}
	}

	UGfpmActionObserver_Base* NewObserver = NewObject<UGfpmActionObserver_Base>(this, ObserverClass);
	Observers.Emplace(NewObserver);
	return NewObserver;
}
