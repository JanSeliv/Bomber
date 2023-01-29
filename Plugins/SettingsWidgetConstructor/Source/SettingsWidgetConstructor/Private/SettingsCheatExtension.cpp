// Copyright (c) Yevhenii Selivanov

#include "SettingsCheatExtension.h"
//---
#include "MyUtilsLibraries/SWCWidgetUtilsLibrary.h"
#include "UI/SettingsWidget.h"

// Default constructor
USettingsCheatExtension::USettingsCheatExtension()
{
	UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateUObject(this, &ThisClass::OnCheatManagerCreated));
}

// Register a delegate to call whenever a cheat manager is spawned; it will also be called immediately for cheat managers that already exist at this point
void USettingsCheatExtension::OnCheatManagerCreated(UCheatManager* CheatManager)
{
	if (!HasAllFlags(RF_ClassDefaultObject) // Continue if only is CDO object
		|| !CheatManager)
	{
		return;
	}

	// Spawn the extension from its CDO and add it to the cheat manager
	USettingsCheatExtension* Extension = NewObject<ThisClass>(CheatManager);
	CheatManager->AddCheatManagerExtension(Extension);
}

// Override the setting value with the cheat
void USettingsCheatExtension::CheatSetting(const FString& TagByValue) const
{
	USettingsWidget* SettingsWidget = FSWCWidgetUtilsLibrary::FindWidgetOfClass<USettingsWidget>(GetWorld());
	if (!SettingsWidget)
	{
		return;
	}

	if (TagByValue.IsEmpty())
	{
		return;
	}

	static const FString Delimiter = TEXT("?");
	TArray<FString> SeparatedStrings;
	TagByValue.ParseIntoArray(SeparatedStrings, *Delimiter);

	static constexpr int32 TagIndex = 0;
	FName TagName = NAME_None;
	if (SeparatedStrings.IsValidIndex(TagIndex))
	{
		TagName = *SeparatedStrings[TagIndex];
	}

	if (TagName.IsNone())
	{
		return;
	}

	// Extract value
	static constexpr int32 ValueIndex = 1;
	FString TagValue = TEXT("");
	if (SeparatedStrings.IsValidIndex(ValueIndex))
	{
		TagValue = SeparatedStrings[ValueIndex];
	}

	SettingsWidget->SetSettingValue(TagName, TagValue);
	SettingsWidget->SaveSettings();
}
