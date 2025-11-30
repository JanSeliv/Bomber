// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMInGameSettingsSubsystem.h"

// NMM
#include "Data/NMMDataAsset.h"
#include "NMMUtils.h"

// Bomber
#include "Subsystems/BmrSoundsSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMInGameSettingsSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNMMInGameSettingsSubsystem& UNMMInGameSettingsSubsystem::Get()
{
	UNMMInGameSettingsSubsystem* Subsystem = UNMMUtils::GetInGameSettingsSubsystem();
	checkf(Subsystem, TEXT("%s: 'NMMInGameSettingsSubsystem' is null"), *FString(__FUNCTION__));
	return *Subsystem;
}

// Set to skips previously seen cinematics automatically
void UNMMInGameSettingsSubsystem::SetAutoSkipCinematics(bool bEnable)
{
	bAutoSkipCinematics = bEnable;
}

// Set new sound volume for Cinematics sound class
void UNMMInGameSettingsSubsystem::SetCinematicsVolume(double InVolume)
{
	CinematicsVolume = InVolume;

	USoundClass* CinematicsSoundClass = UNMMDataAsset::Get().GetCinematicsSoundClass();
	UBmrSoundsSubsystem::Get().SetSoundVolumeByClass(CinematicsSoundClass, InVolume);
}

// Set true to enable instant transitions when switching characters in the Main Menu
void UNMMInGameSettingsSubsystem::SetInstantCharacterSwitchEnabled(bool bEnable)
{
	bInstantCharacterSwitch = bEnable;
}
