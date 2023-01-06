// Copyright (c) Yevhenii Selivanov

#pragma once

#include "SettingTypes.generated.h"

/**	┌───────────────────────────┐
  *	│			[SETTINGS]		│ Header (title)
  *	│	[Option 1]	[Option 2]	│ Content (options)
  *	│			[GO BACK]		│ Footer
  *	└───────────────────────────┘ */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMyVerticalAlignment : uint8
{
	None = 0 UMETA(Hidden),
	Header = 1 << 0,
	Content = 1 << 1,
	Footer = 1 << 2,
	Margins = Header | Footer UMETA(Hidden),
	All = Header | Content | Footer UMETA(Hidden)
};

ENUM_CLASS_FLAGS(EMyVerticalAlignment)

/**
  * All UI states of the button.
  */
UENUM(BlueprintType)
enum class ESettingsButtonState : uint8
{
	Normal,
	Hovered,
	Pressed,
	Disabled
};

/**
  * All UI states of the checkbox.
  */
UENUM(BlueprintType)
enum class ESettingsCheckboxState : uint8
{
	UncheckedNormal,
	UncheckedHovered,
	UncheckedPressed,
	CheckedNormal,
	CheckedHovered,
	CheckedPressed,
	UndeterminedNormal,
	UndeterminedHovered,
	UndeterminedPressed
};

/**
  * All UI states of the slider.
  */
UENUM(BlueprintType)
enum class ESettingsSliderState : uint8
{
	NormalBar,
	HoveredBar,
	NormalThumb,
	HoveredThumb
};
