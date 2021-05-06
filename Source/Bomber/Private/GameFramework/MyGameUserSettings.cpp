// Copyright 2021 Yevhenii Selivanov.

#include "GameFramework/MyGameUserSettings.h"
//---
#include "Globals/SingletonLibrary.h"
//---
#include "Engine/DataTable.h"

#if WITH_EDITOR //[include]
#include "DataTableEditorUtils.h"
#endif // WITH_EDITOR

// Returns the settings data asset
const USettingsDataAsset& USettingsDataAsset::Get()
{
	const USettingsDataAsset* SettingsDataAsset = USingletonLibrary::GetSettingsDataAsset();
	checkf(SettingsDataAsset, TEXT("The Settings Data Asset is not valid"));
	return *SettingsDataAsset;
}

// Returns the table rows.
void USettingsDataAsset::GenerateSettingsArray(TMap<FName, FSettingsPicker>& OutRows) const
{
	if (!ensureMsgf(SettingsDataTableInternal, TEXT("ASSERT: 'SettingsDataTableInternal' is not valid")))
	{
		return;
	}

	const TMap<FName, uint8*>& RowMap = SettingsDataTableInternal->GetRowMap();
	OutRows.Empty();
	OutRows.Reserve(RowMap.Num());
	for (const TTuple<FName, uint8*>& RowIt : RowMap)
	{
		if (const auto FoundRowPtr = reinterpret_cast<FSettingsRow*>(RowIt.Value))
		{
			const FSettingsPicker& SettingsTableRow = FoundRowPtr->SettingsPicker;
			const FName RowName = RowIt.Key;
			OutRows.Emplace(RowName, SettingsTableRow);
		}
	}
}

// Get a multicast delegate that is called any time the data table changes
void USettingsDataAsset::BindOnDataTableChanged(const FOnDataTableChanged& EventToBind) const
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (!USingletonLibrary::IsEditorNotPieWorld()
	    || !SettingsDataTableInternal
	    || !EventToBind.IsBound())
	{
		return;
	}

	UDataTable::FOnDataTableChanged& OnDataTableChangedDelegate = SettingsDataTableInternal->OnDataTableChanged();
	OnDataTableChangedDelegate.AddLambda([EventToBind]() { EventToBind.ExecuteIfBound(); });
#endif // WITH_EDITOR
}

// Returns the game user settings
UMyGameUserSettings& UMyGameUserSettings::Get()
{
	UMyGameUserSettings* MyGameUserSettings = USingletonLibrary::GetMyGameUserSettings();
	checkf(MyGameUserSettings, TEXT("My Game User Settings is not valid"));
	return *MyGameUserSettings;
}

// Get all supported resolutions of the primary monitor
void UMyGameUserSettings::UpdateSupportedResolutions()
{
	FScreenResolutionArray ResolutionsArray;
	const bool bWasFound = RHIGetAvailableResolutions(ResolutionsArray, true);
	if (!bWasFound)
	{
		return;
	}

	TextResolutionsInternal.Empty();
	IntResolutionsInternal.Empty();

	const FIntPoint PrimaryDisplayNativeResolution = GetDesktopResolution();
	const int32 MaxDisplayWidth = PrimaryDisplayNativeResolution.X;
	const int32 MaxDisplayHeight = PrimaryDisplayNativeResolution.Y;
	const float AspectRatio = FMath::DivideAndRoundDown<float>(MaxDisplayWidth, MaxDisplayHeight);

	const int32 ResolutionsArrayNum = ResolutionsArray.Num();
	for (int32 Index = ResolutionsArrayNum - 1; Index >= 0; --Index)
	{
		if (!ResolutionsArray.IsValidIndex(Index))
		{
			continue;
		}

		const FScreenResolutionRHI& ResolutionIt = ResolutionsArray[Index];
		const int32 WidthIt = ResolutionIt.Width;
		const int32 HeightIt = ResolutionIt.Height;
		const float AspectRatioIt = FMath::DivideAndRoundDown<float>(WidthIt, HeightIt);

		const bool bIsSameAspectRatio = FMath::IsNearlyEqual(AspectRatioIt, AspectRatio);
		const bool bIsGreaterThanMin = WidthIt >= MinResolutionSizeXInternal
		                               && HeightIt >= MinResolutionSizeYInternal;
		const bool bIsLessThanMax = WidthIt <= MaxDisplayWidth
		                            && HeightIt <= MaxDisplayHeight;

		if (!bIsSameAspectRatio
		    || !bIsGreaterThanMin
		    || !bIsLessThanMax)
		{
			continue;
		}

		static const FString Delimiter = TEXT("x");
		FText TextResolution = FText::FromString(FString::FromInt(WidthIt) + Delimiter + FString::FromInt(HeightIt));
		TextResolutionsInternal.Emplace(MoveTemp(TextResolution));

		FIntPoint IntResolution(WidthIt, HeightIt);
		const int32 AddedIndex = IntResolutionsInternal.Emplace(MoveTemp(IntResolution));

		if (WidthIt == ResolutionSizeX
		    && HeightIt == ResolutionSizeY)
		{
			CurrentResolutionIndexInternal = AddedIndex;
		}
	}
}

// Set new resolution by index
void UMyGameUserSettings::SetResolutionByIndex(int32 Index)
{
	if (!IntResolutionsInternal.IsValidIndex(Index))
	{
		return;
	}

	const FIntPoint& NewResolution = IntResolutionsInternal[Index];
	SetScreenResolution(NewResolution);
	ApplyResolutionSettings(true);

	CurrentResolutionIndexInternal = Index;
}

// Set and apply fullscreen mode. If false, the windowed mode will be applied
void UMyGameUserSettings::SetFullscreenEnabled(bool bIsFullscreen)
{
	const EWindowMode::Type NewFullscreenMode = bIsFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed;
	SetFullscreenMode(NewFullscreenMode);
	ApplyResolutionSettings(true);
}

// Loads the user settings from persistent storage
void UMyGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);

	if (!IntResolutionsInternal.Num())
	{
		UpdateSupportedResolutions();
	}

#if WITH_EDITOR // [IsEditorNotPieWorld]
	// Notify settings for any change in the settings data table
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Bind only once
		static USettingsDataAsset::FOnDataTableChanged OnDataTableChanged;
		if (!OnDataTableChanged.IsBound())
		{
			OnDataTableChanged.BindDynamic(this, &ThisClass::OnDataTableChanged);
			USettingsDataAsset::Get().BindOnDataTableChanged(OnDataTableChanged);
		}
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// Called whenever the data of a table has changed, this calls the OnDataTableChanged() delegate and per-row callbacks
void UMyGameUserSettings::OnDataTableChanged()
{
#if WITH_EDITOR  // [IsEditorNotPieWorld]
	if (!USingletonLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	UDataTable* SettingsDataTable = USettingsDataAsset::Get().GetSettingsDataTable();
	if (!ensureMsgf(SettingsDataTable, TEXT("ASSERT: 'SettingsDataTable' is not valid")))
	{
		return;
	}

	// Set row name by specified tag
	TMap<FName, FSettingsPicker> SettingsArray;
	USettingsDataAsset::Get().GenerateSettingsArray(SettingsArray);
	for (const TTuple<FName, FSettingsPicker>& SettingsTableRowIt : SettingsArray)
	{
		const FSettingsPicker& SettingsRow = SettingsTableRowIt.Value;

		const FName RowKey = SettingsTableRowIt.Key;
		const FName RowValueTag = SettingsTableRowIt.Value.PrimaryData.Tag.GetTagName();
		if (!RowValueTag.IsNone()                    // Tag is not empty
		    && RowKey != RowValueTag                 // New tag name
		    && !SettingsArray.Contains(RowValueTag)) // Unique tag
		{
			FDataTableEditorUtils::RenameRow(SettingsDataTable, RowKey, RowValueTag);
		}
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}
