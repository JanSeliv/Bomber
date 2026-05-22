// Copyright (c) Yevhenii Selivanov

#include "ActionObservers/GfpmActionObserver_Base.h"

// UE
#include "GameFeatureAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmActionObserver_Base)

// Binds this observer to its observed action
void UGfpmActionObserver_Base::SetObservedAction(UGameFeatureAction* Action, const FString& InPluginURL)
{
	ObservedAction = Action;
	PluginURL = InPluginURL;
}

// Whether this observer is created in current configuration, by default only in editor and not under -game
bool UGfpmActionObserver_Base::ShouldCreateObserver() const
{
	return GIsEditor;
}
