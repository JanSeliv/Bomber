// Copyright (c) Yevhenii Selivanov.

#include "GameFramework/MyGameUserSettings.h"
//---
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "DynamicRHI.h"
#include "Misc/ConfigCacheIni.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyGameUserSettings)

// Returns the game user settings
UMyGameUserSettings& UMyGameUserSettings::Get()
{
	UMyGameUserSettings* MyGameUserSettings = UMyBlueprintFunctionLibrary::GetMyGameUserSettings();
	checkf(MyGameUserSettings, TEXT("My Game User Settings is not valid"));
	return *MyGameUserSettings;
}

/*********************************************************************************************
 * Resolutions
 ********************************************************************************************* */

// Get all supported resolutions of the primary monitor
void UMyGameUserSettings::UpdateSupportedResolutions()
{
	FScreenResolutionArray ResolutionsArray;
	const bool bWasFound = RHIGetAvailableResolutions(ResolutionsArray, true);
	if (!bWasFound)
	{
		return;
	}

	FIntPoint MaxDisplayResolution = FIntPoint::ZeroValue;
	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
	for (const FMonitorInfo& MonitorIt : DisplayMetrics.MonitorInfo)
	{
		if (MonitorIt.bIsPrimary)
		{
			const FIntPoint NativeRes(MonitorIt.NativeWidth, MonitorIt.NativeHeight);
			const FIntPoint WorkAreaRes(MonitorIt.WorkArea.Right - MonitorIt.WorkArea.Left, MonitorIt.WorkArea.Bottom - MonitorIt.WorkArea.Top);
			const FIntPoint DisplayRectRes(MonitorIt.DisplayRect.Right - MonitorIt.DisplayRect.Left, MonitorIt.DisplayRect.Bottom - MonitorIt.DisplayRect.Top);
			MaxDisplayResolution = NativeRes.ComponentMax(WorkAreaRes).ComponentMax(DisplayRectRes).ComponentMax(MonitorIt.MaxResolution);
			break;
		}
	}

	if (MaxDisplayResolution == FIntPoint::ZeroValue)
	{
		return;
	}

	const float MaxDisplayWidth = static_cast<float>(MaxDisplayResolution.X);
	const float MaxDisplayHeight = static_cast<float>(MaxDisplayResolution.Y);
	const float AspectRatio = FMath::DivideAndRoundDown<float>(MaxDisplayWidth, MaxDisplayHeight);

	bool bIsUltraWide = false;
	constexpr float UltraWideAspectRatio = 21.f / 9.f;
	if (AspectRatio >= UltraWideAspectRatio)
	{
		bIsUltraWide = true;
	}

	TextResolutionsInternal.Empty();
	IntResolutionsInternal.Empty();
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

		const bool bIsCorrectAspectRatio = FMath::IsNearlyEqual(AspectRatioIt, AspectRatio)
		                                   || (bIsUltraWide && AspectRatioIt >= UltraWideAspectRatio);
		const bool bIsGreaterThanMin = WidthIt >= MinResolutionSizeXInternal
		                               && HeightIt >= MinResolutionSizeYInternal;
		const bool bIsLessThanMax = WidthIt <= MaxDisplayWidth
		                            && HeightIt <= MaxDisplayHeight;
		const bool bIsEightDivisible = WidthIt % 8 == 0 && HeightIt % 8 == 0;

		if (!bIsCorrectAspectRatio
		    || !bIsGreaterThanMin
		    || !bIsLessThanMax
		    || !bIsEightDivisible)
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
	if (!IntResolutionsInternal.IsValidIndex(Index)
	    || GetResolutionIndex() == Index)
	{
		return;
	}

	const FIntPoint& NewResolution = IntResolutionsInternal[Index];
	SetScreenResolution(NewResolution);

	CurrentResolutionIndexInternal = Index;
	LastUserConfirmedResolutionSizeX = NewResolution.X;
	LastUserConfirmedResolutionSizeY = NewResolution.Y;

	const bool bIsMaxRes = Index == 0;
	if (IsFullscreenEnabled() != bIsMaxRes)
	{
		// Update fullscreen mode to match resolution
		SetFullscreenEnabled(bIsMaxRes);
	}

	ApplyResolutionSettings(false);
}

// Syncs the current resolution index with the actual resolution, is useful when it's changed outside (e.g. by Alt+Enter).
void UMyGameUserSettings::TryUpdateCurrentResolution()
{
	const bool bIsResolutionChangedOutside = LastUserConfirmedResolutionSizeX != ResolutionSizeX
	                                         || LastUserConfirmedResolutionSizeY != ResolutionSizeY;
	if (!bIsResolutionChangedOutside)
	{
		// Resolution is already up to date
		return;
	}

	const int32 NewResolutionIndex = IntResolutionsInternal.IndexOfByPredicate([&](const FIntPoint& ResolutionIt)
	{
		return ResolutionIt.X == ResolutionSizeX
		       && ResolutionIt.Y == ResolutionSizeY;
	});

	if (NewResolutionIndex != INDEX_NONE)
	{
		SetResolutionByIndex(NewResolutionIndex);
	}

	// Finally, try update the setting on UI
	if (USettingsWidget* SettingsWidget = UMyBlueprintFunctionLibrary::GetSettingsWidget())
	{
		static FGameplayTagContainer ResolutionSettingTag = FGameplayTagContainer::EmptyContainer;
		if (ResolutionSettingTag.IsEmpty())
		{
			static const FSettingFunctionPicker SetResolutionFunction(GetClass(), GET_FUNCTION_NAME_CHECKED(ThisClass, SetResolutionByIndex));
			ResolutionSettingTag.AddTag(SettingsWidget->GetTagByFunction(SetResolutionFunction));
		}

		SettingsWidget->UpdateSettings(ResolutionSettingTag);
	}
}

/*********************************************************************************************
 * Fullscreen
 ********************************************************************************************* */

// Set and apply fullscreen mode. If false, the windowed mode will be applied
void UMyGameUserSettings::SetFullscreenEnabled(bool bIsFullscreen)
{
	if (IsFullscreenEnabled() == bIsFullscreen)
	{
		return;
	}

	const EWindowMode::Type NewFullscreenMode = GetSupportedWindowModeType(bIsFullscreen);
	SetFullscreenMode(NewFullscreenMode);

	LastConfirmedFullscreenMode = NewFullscreenMode;

	const bool bIsMaxRes = CurrentResolutionIndexInternal == 0;
	if (bIsFullscreen != bIsMaxRes)
	{
		// User enabled fullscreen: set max resolution
		// User disabled fullscreen: set Max+1 resolution
		constexpr int32 MaxResolutionIndex = 0;
		constexpr int32 PrevResolutionIndex = MaxResolutionIndex + 1;
		const int32 NewResolutionIndex = bIsFullscreen ? MaxResolutionIndex : PrevResolutionIndex;
		SetResolutionByIndex(NewResolutionIndex);
	}

	ApplyResolutionSettings(false);
}

// Syncs the current Fullscreen mode with the actual mode, is useful when it's changed outside (e.g. by Alt+Enter)
void UMyGameUserSettings::TryUpdateCurrentFullscreenMode()
{
	if (LastConfirmedFullscreenMode == FullscreenMode)
	{
		// Fullscreen mode is already up to date
		return;
	}

	const bool bIsMaxRes = CurrentResolutionIndexInternal == 0;
	if (IsFullscreenEnabled() && !bIsMaxRes)
	{
		// Update resolution to maximum to enter in fullscreen
		constexpr int32 MaxResolutionIndex = 0;
		SetResolutionByIndex(MaxResolutionIndex);
	}

	// Finally, try update the setting on UI
	if (USettingsWidget* SettingsWidget = UMyBlueprintFunctionLibrary::GetSettingsWidget())
	{
		static FGameplayTagContainer FullscreenSettingTag = FGameplayTagContainer::EmptyContainer;
		if (FullscreenSettingTag.IsEmpty())
		{
			static const FSettingFunctionPicker SetFullscreenFunction(GetClass(), GET_FUNCTION_NAME_CHECKED(ThisClass, SetFullscreenEnabled));
			FullscreenSettingTag.AddTag(SettingsWidget->GetTagByFunction(SetFullscreenFunction));
		}

		SettingsWidget->UpdateSettings(FullscreenSettingTag);
	}
}

/*********************************************************************************************
 * FPS Lock
 ********************************************************************************************* */

// Set the FPS cap by specified member index
void UMyGameUserSettings::SetFPSLockByIndex(int32 Index)
{
	const USettingsWidget* SettingsWidget = UMyBlueprintFunctionLibrary::GetSettingsWidget();
	if (!SettingsWidget
	    || GetFPSLockIndex() == Index)
	{
		return;
	}

	static const FSettingFunctionPicker ThisFunction(GetClass(), GET_FUNCTION_NAME_CHECKED(ThisClass, SetFPSLockByIndex));
	const FSettingTag& FPSLockTag = SettingsWidget->GetTagByFunction(ThisFunction);
	if (!FPSLockTag.IsValid())
	{
		return;
	}

	TArray<FText> ComboboxMembers;
	SettingsWidget->GetComboboxMembers(FPSLockTag, ComboboxMembers);
	if (!ComboboxMembers.IsValidIndex(Index))
	{
		return;
	}

	auto SetFPSLock = [&](int32 MaxFPS)
	{
		// 0 disables frame rate limiting
		SetFrameRateLimit(MaxFPS);
		SetFrameRateLimitCVar(MaxFPS);

		FPSLockIndexInternal = Index;
	};

	// If numeric like '144', then set and return
	const FString& StrMaxFPS = ComboboxMembers[Index].ToString();
	if (StrMaxFPS.IsNumeric())
	{
		const int32 MaxFPS = FCString::Atoi(*StrMaxFPS);
		SetFPSLock(MaxFPS);
		return;
	}

	// If numeric contains in the string like '144 FPS', then extract '144', set and return
	static const FString SpaceDelimiter = TEXT(" ");
	TArray<FString> StringArray;
	StrMaxFPS.ParseIntoArray(StringArray, *SpaceDelimiter);
	const FString* FoundNumericStr = StringArray.FindByPredicate([](const FString& StrIt) { return StrIt.IsNumeric(); });
	if (FoundNumericStr)
	{
		const int32 MaxFPS = FCString::Atoi(**FoundNumericStr);
		SetFPSLock(MaxFPS);
		return;
	}

	// It is not a numeric member, uncap FPS
	static constexpr int32 UncappedFPS = 0;
	SetFPSLock(UncappedFPS);
}

/*********************************************************************************************
 * Overall Quality (Scalability)
 ********************************************************************************************* */

// Changes all scalability settings at once based on a single overall quality level
void UMyGameUserSettings::SetOverallScalabilityLevel(int32 Value)
{
	if (Value == OverallQualityInternal)
	{
		return;
	}

	OverallQualityInternal = Value;

	if (!OverallQualityInternal)
	{
		// Custom scalability is set
		return;
	}

	static constexpr int32 QualityOffset = 1;
	Super::SetOverallScalabilityLevel(Value - QualityOffset);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Loads the user settings from persistent storage
void UMyGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);

	const bool bUpdateMinResolutions = !MinResolutionSizeXInternal || !MinResolutionSizeYInternal;
	if (bUpdateMinResolutions
	    && GConfig
	    && !GGameIni.IsEmpty())
	{
		static const FString Section(TEXT("/Script/EngineSettings.GeneralProjectSettings"));
		GConfig->GetInt(*Section, TEXT("MinWindowWidth"), MinResolutionSizeXInternal, GGameIni);
		GConfig->GetInt(*Section, TEXT("MinWindowHeight"), MinResolutionSizeYInternal, GGameIni);
	}

	if (!IntResolutionsInternal.Num())
	{
		UpdateSupportedResolutions();
	}

	constexpr float NoBenchmarkRun = -1.f;
	if (GetLastGPUBenchmarkResult() == NoBenchmarkRun)
	{
		RunHardwareBenchmark();
		ApplyHardwareBenchmarkResults();
	}
}

// Validates and resets bad user settings to default. Deletes stale user settings file if necessary
void UMyGameUserSettings::ValidateSettings()
{
	Super::ValidateSettings();

	// Validate resolution
	if (IntResolutionsInternal.IsValidIndex(CurrentResolutionIndexInternal))
	{
		const FIntPoint ChosenScreenResolution(IntResolutionsInternal[CurrentResolutionIndexInternal]);
		const FIntPoint CurrentScreenResolution(GetScreenResolution());
		if (ChosenScreenResolution != CurrentScreenResolution)
		{
			SetResolutionByIndex(CurrentResolutionIndexInternal);
		}
	}
}

// Save the user settings to persistent storage (automatically happens as part of ApplySettings)
void UMyGameUserSettings::SaveSettings()
{
	Super::SaveSettings();

	if (OnSaveSettings.IsBound())
	{
		OnSaveSettings.Broadcast();
	}
}

// Returns the overall scalability level
int32 UMyGameUserSettings::GetOverallScalabilityLevel() const
{
	static constexpr int32 QualityOffset = 1;
	const int32 OverallScalabilityLevel = Super::GetOverallScalabilityLevel();
	return OverallScalabilityLevel + QualityOffset;
}

// Mark current video mode settings (fullscreenmode/resolution) as being confirmed by the user
void UMyGameUserSettings::ConfirmVideoMode()
{
	TryUpdateCurrentResolution();

	TryUpdateCurrentFullscreenMode();

	Super::ConfirmVideoMode();
}