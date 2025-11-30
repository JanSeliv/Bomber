// Copyright (c) Yevhenii Selivanov.

#include "GameFramework/BmrGameUserSettings.h"

// Bomber
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "DynamicRHI.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ConfigCacheIni.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameUserSettings)

// Returns the game user settings
UBmrGameUserSettings& UBmrGameUserSettings::Get()
{
	UBmrGameUserSettings* MyGameUserSettings = UBmrBlueprintFunctionLibrary::GetGameUserSettings();
	checkf(MyGameUserSettings, TEXT("My Game User Settings is not valid"));
	return *MyGameUserSettings;
}

/*********************************************************************************************
 * Resolutions
 ********************************************************************************************* */

// Get all supported resolutions of the primary monitor
void UBmrGameUserSettings::UpdateSupportedResolutions()
{
	if (UUtilsLibrary::IsEditor())
	{
		// In editor, engine never applies resolutions
		static const FText EditorMsg = FText::FromString(TEXT("NO EDITOR SUPPORT"));
		TextResolutions.Empty();
		TextResolutions.Emplace(EditorMsg);
		return;
	}

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

	TextResolutions.Empty();
	IntResolutions.Empty();
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
		const bool bIsGreaterThanMin = WidthIt >= MinResolutionSizeX
		                               && HeightIt >= MinResolutionSizeY;
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
		TextResolutions.Emplace(MoveTemp(TextResolution));

		FIntPoint IntResolution(WidthIt, HeightIt);
		const int32 AddedIndex = IntResolutions.Emplace(MoveTemp(IntResolution));

		if (WidthIt == ResolutionSizeX
		    && HeightIt == ResolutionSizeY)
		{
			CurrentResolutionIndex = AddedIndex;
		}
	}
}

// Set new resolution by index
void UBmrGameUserSettings::SetResolutionByIndex(int32 Index)
{
	if (!IntResolutions.IsValidIndex(Index)
	    || GetResolutionIndex() == Index
	    || UUtilsLibrary::IsEditor())
	{
		// Is not supported (editor)
		return;
	}

	const FIntPoint& NewResolution = IntResolutions[Index];
	SetScreenResolution(NewResolution);

	CurrentResolutionIndex = Index;
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
void UBmrGameUserSettings::TryUpdateCurrentResolution()
{
	const bool bIsResolutionChangedOutside = LastUserConfirmedResolutionSizeX != ResolutionSizeX
	                                         || LastUserConfirmedResolutionSizeY != ResolutionSizeY;
	if (!bIsResolutionChangedOutside)
	{
		// Resolution is already up to date
		return;
	}

	const int32 NewResolutionIndex = IntResolutions.IndexOfByPredicate([&](const FIntPoint& ResolutionIt)
	{
		return ResolutionIt.X == ResolutionSizeX
		       && ResolutionIt.Y == ResolutionSizeY;
	});

	if (NewResolutionIndex != INDEX_NONE)
	{
		SetResolutionByIndex(NewResolutionIndex);
	}

	// Finally, try update the setting on UI
	UPDATE_SETTING_BY_FUNCTION(UBmrBlueprintFunctionLibrary::GetSettingsWidget(), ThisClass, SetResolutionByIndex);
}

/*********************************************************************************************
 * Fullscreen
 ********************************************************************************************* */

// Set and apply fullscreen mode. If false, the windowed mode will be applied
void UBmrGameUserSettings::SetFullscreenEnabled(bool bIsFullscreen)
{
	if (IsFullscreenEnabled() == bIsFullscreen)
	{
		return;
	}

	const EWindowMode::Type NewFullscreenMode = GetSupportedWindowModeType(bIsFullscreen);
	SetFullscreenMode(NewFullscreenMode);

	LastConfirmedFullscreenMode = NewFullscreenMode;

	const bool bIsMaxRes = CurrentResolutionIndex == 0;
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
void UBmrGameUserSettings::TryUpdateCurrentFullscreenMode()
{
	if (LastConfirmedFullscreenMode == FullscreenMode)
	{
		// Fullscreen mode is already up to date
		return;
	}

	const bool bIsMaxRes = CurrentResolutionIndex == 0;
	if (IsFullscreenEnabled() && !bIsMaxRes)
	{
		// Update resolution to maximum to enter in fullscreen
		constexpr int32 MaxResolutionIndex = 0;
		SetResolutionByIndex(MaxResolutionIndex);
	}

	// Finally, try update the setting on UI
	UPDATE_SETTING_BY_FUNCTION(UBmrBlueprintFunctionLibrary::GetSettingsWidget(), ThisClass, SetFullscreenEnabled);
}

/*********************************************************************************************
 * FPS Lock
 ********************************************************************************************* */

// Set the FPS cap by specified member index
void UBmrGameUserSettings::SetFPSLockByIndex(int32 Index)
{
	const USettingsWidget* SettingsWidget = UBmrBlueprintFunctionLibrary::GetSettingsWidget();
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

		FPSLockIndex = Index;
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
	const FString* FoundNumericStr = StringArray.FindByPredicate([](const FString& StrIt)
	{
		return StrIt.IsNumeric();
	});
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
void UBmrGameUserSettings::SetOverallScalabilityLevel(int32 Value)
{
	if (Value == OverallQuality)
	{
		return;
	}

	OverallQuality = Value;
	ApplyOverallScalabilityLevel();
}

// Is called to apply the currently chosen overall quality level
void UBmrGameUserSettings::ApplyOverallScalabilityLevel()
{
	if (!OverallQuality)
	{
		// Custom scalability is set
		return;
	}

	static constexpr int32 QualityOffset = 1;
	Super::SetOverallScalabilityLevel(OverallQuality - QualityOffset);

	SetQualityLevels(ScalabilityQuality);
}

/*********************************************************************************************
 * Language
 ********************************************************************************************* */

// Set new language by index
void UBmrGameUserSettings::SetLanguageByIndex(int32 Index)
{
	if (!Cultures.IsValidIndex(Index)
	    || UUtilsLibrary::IsEditor())
	{
		// Is not supported (editor)
		return;
	}

	AppliedCulture = Cultures[Index];
	CurrentLanguageIndex = Index;
	ApplyCurrentLanguage();
}

// Is called to apply the currently chosen language
void UBmrGameUserSettings::ApplyCurrentLanguage()
{
	const bool bIsApplied = FInternationalization::Get().SetCurrentLanguageAndLocale(AppliedCulture.ToString());
	if (!bIsApplied)
	{
		return;
	}

	SaveSettings();
}

// Get all supported languages
void UBmrGameUserSettings::UpdateSupportedLanguages()
{
	if (UUtilsLibrary::IsEditor())
	{
		// In editor, engine never applies cultures
		static const FText EditorMsg = FText::FromString(TEXT("NO EDITOR SUPPORT"));
		DisplayLanguages.Empty();
		DisplayLanguages.Emplace(EditorMsg);
		return;
	}

	// Get all available cultures
	const FTextLocalizationManager& LocalizationManager = FTextLocalizationManager::Get();
	const TArray<FString> AllAvailableCultures = LocalizationManager.GetLocalizedCultureNames(ELocalizationLoadFlags::Game);
	if (AllAvailableCultures.IsEmpty())
	{
		// No localization data found
		return;
	}

	// Always add English by default as fallback language to display first if system language is not supported
	Cultures.Empty();
	static const FString FallbackDefaultLanguage = TEXT("en");
	Cultures.Emplace(FallbackDefaultLanguage);

	// Make the system (OS) language to be displayed first, if supported: it contains within available cultures
	const FString SystemCulture = UKismetSystemLibrary::GetDefaultLanguage();
	const FString* AvailableSystemCulture = AllAvailableCultures.FindByPredicate([&SystemCulture](const FString& It)
	{
		return SystemCulture.Contains(It);
	});
	if (AvailableSystemCulture
	    && *AvailableSystemCulture != FallbackDefaultLanguage)
	{
		Cultures.Insert(**AvailableSystemCulture, 0);
	}

	// Add all remaining cultures
	for (const FString& AvailableCultureIt : AllAvailableCultures)
	{
		Cultures.AddUnique(*AvailableCultureIt);
	}

	// Convert culture names (en) to localized language names (English) to be displayed on UI, in their translated form
	DisplayLanguages.Empty();
	for (const FName LocalizedCultureNameIt : Cultures)
	{
		constexpr bool bLocalized = false;
		const FString LocalizedCultureName = UKismetInternationalizationLibrary::GetCultureDisplayName(LocalizedCultureNameIt.ToString(), bLocalized);
		ensureAlwaysMsgf(LocalizedCultureName.Len() == FCString::Strlen(*LocalizedCultureName), TEXT("ASSERT: [%i] %hs:\n'%s' culture name is not supported!"), __LINE__, __FUNCTION__, *LocalizedCultureName);
		DisplayLanguages.Emplace(FText::FromString(LocalizedCultureName));
	}

	// If very first launch of the game, then set the system language as default
	if (AppliedCulture.IsNone())
	{
		// Set the system language as default
		checkf(Cultures.IsValidIndex(0), TEXT("ERROR: [%i] %hs:\n'Cultures.IsValidIndex(0)' is null!"), __LINE__, __FUNCTION__);
		AppliedCulture = Cultures[0];
	}

	// Always update current language index
	if (CurrentLanguageIndex == INDEX_NONE)
	{
		CurrentLanguageIndex = Cultures.IndexOfByKey(AppliedCulture);
	}
}

/*********************************************************************************************
 * FPS Counter
 ********************************************************************************************* */

// Set true to show the FPS counter widget on the HUD
void UBmrGameUserSettings::SetFPSCounterEnabled(bool bEnable)
{
	const UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem();
	UUserWidget* FPSCounterWidget = WidgetsSubsystem ? WidgetsSubsystem->GetWidgetByTag(BmrGameplayTags::UI::Widget_FpsCounter) : nullptr;
	if (ensureMsgf(FPSCounterWidget, TEXT("ASSERT: [%i] %hs:\n'FPSCounterWidget' was not found!"), __LINE__, __FUNCTION__))
	{
		const ESlateVisibility NewVisibility = bEnable ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		FPSCounterWidget->SetVisibility(NewVisibility);
		bIsFPSCounterEnabled = bEnable;
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Loads the user settings from persistent storage
void UBmrGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);

	const bool bUpdateMinResolutions = !MinResolutionSizeX || !MinResolutionSizeY;
	if (bUpdateMinResolutions
	    && GConfig
	    && !GGameIni.IsEmpty())
	{
		static const FString Section(TEXT("/Script/EngineSettings.GeneralProjectSettings"));
		GConfig->GetInt(*Section, TEXT("MinWindowWidth"), MinResolutionSizeX, GGameIni);
		GConfig->GetInt(*Section, TEXT("MinWindowHeight"), MinResolutionSizeY, GGameIni);
	}

	if (IntResolutions.IsEmpty())
	{
		UpdateSupportedResolutions();
	}

	if (DisplayLanguages.IsEmpty()
	    || Cultures.IsEmpty()
	    || CurrentLanguageIndex < 0)
	{
		UpdateSupportedLanguages();
		ApplyCurrentLanguage();
	}

	ApplyOverallScalabilityLevel();
}

// Validates and resets bad user settings to default. Deletes stale user settings file if necessary
void UBmrGameUserSettings::ValidateSettings()
{
	Super::ValidateSettings();

	// Validate resolution
	if (IntResolutions.IsValidIndex(CurrentResolutionIndex))
	{
		const FIntPoint ChosenScreenResolution(IntResolutions[CurrentResolutionIndex]);
		const FIntPoint CurrentScreenResolution(GetScreenResolution());
		if (ChosenScreenResolution != CurrentScreenResolution)
		{
			SetResolutionByIndex(CurrentResolutionIndex);
		}
	}
}

// Save the user settings to persistent storage (automatically happens as part of ApplySettings)
void UBmrGameUserSettings::SaveSettings()
{
	Super::SaveSettings();

	if (OnSaveSettings.IsBound())
	{
		OnSaveSettings.Broadcast();
	}
}

// Returns the overall scalability level
int32 UBmrGameUserSettings::GetOverallScalabilityLevel() const
{
	static constexpr int32 QualityOffset = 1;
	const int32 OverallScalabilityLevel = Super::GetOverallScalabilityLevel();
	return OverallScalabilityLevel + QualityOffset;
}

// Mark current video mode settings (fullscreenmode/resolution) as being confirmed by the user
void UBmrGameUserSettings::ConfirmVideoMode()
{
	TryUpdateCurrentResolution();

	TryUpdateCurrentFullscreenMode();

	Super::ConfirmVideoMode();
}