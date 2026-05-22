// Copyright (c) Yevhenii Selivanov

#include "Subsystems/GfpmLoaderEditorSubsystem.h"

// GFPM
#include "GfpmUtils.h"
#include "Subsystems/GfpmLoaderSubsystem.h"

// UE
#include "Editor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmLoaderEditorSubsystem)

// When subsystem initializes
void UGfpmLoaderEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BeginPIEHandle = FEditorDelegates::PreBeginPIE.AddUObject(this, &ThisClass::OnPreBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddUObject(this, &ThisClass::OnEndPIE);
}

// When subsystem is destroyed
void UGfpmLoaderEditorSubsystem::Deinitialize()
{
	if (BeginPIEHandle.IsValid())
	{
		FEditorDelegates::PreBeginPIE.Remove(BeginPIEHandle);
		BeginPIEHandle.Reset();
	}
	if (EndPIEHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(EndPIEHandle);
		EndPIEHandle.Reset();
	}
	ActivePluginsSnapshot.Empty();

	Super::Deinitialize();
}

// When PIE session is about to start
void UGfpmLoaderEditorSubsystem::OnPreBeginPIE_Implementation(bool bIsSimulating)
{
	ActivePluginsSnapshot.Empty();

	const UGfpmLoaderSubsystem* RuntimeLoader = UGfpmLoaderSubsystem::GetLoaderSubsystem();
	if (!RuntimeLoader)
	{
		// No editor-world runtime loader yet, nothing to snapshot
		return;
	}

	TArray<FName> AllRegistered;
	RuntimeLoader->GetAllRegisteredPlugins(AllRegistered);
	for (const FName& Plugin : AllRegistered)
	{
		if (UGfpmUtils::IsGameFeaturePluginActive(Plugin))
		{
			ActivePluginsSnapshot.Add(Plugin);
		}
	}
}

// When PIE session has ended
void UGfpmLoaderEditorSubsystem::OnEndPIE_Implementation(bool bIsSimulating)
{
	const UGfpmLoaderSubsystem* RuntimeLoader = UGfpmLoaderSubsystem::GetLoaderSubsystem();
	if (!RuntimeLoader)
	{
		// No editor-world runtime loader to drive restoration through
		ActivePluginsSnapshot.Empty();
		return;
	}

	// Snapshot captured before PIE, restoring it reverses any plugin state mutations PIE introduced regardless of tag-driven aggregate
	TArray<FName> AllRegistered;
	RuntimeLoader->GetAllRegisteredPlugins(AllRegistered);
	TArray<FName> ToActivate;
	TArray<FName> ToDeactivate;
	for (const FName& Plugin : AllRegistered)
	{
		const bool bShouldBeActive = ActivePluginsSnapshot.Contains(Plugin);
		const bool bIsCurrentlyActive = UGfpmUtils::IsGameFeaturePluginActive(Plugin);
		if (bShouldBeActive && !bIsCurrentlyActive)
		{
			ToActivate.Add(Plugin);
		}
		else if (!bShouldBeActive && bIsCurrentlyActive)
		{
			ToDeactivate.Add(Plugin);
		}
	}
	UGfpmUtils::SetGameFeaturePluginsActive(false, ToDeactivate);
	UGfpmUtils::SetGameFeaturePluginsActive(true, ToActivate);
	ActivePluginsSnapshot.Empty();
}
