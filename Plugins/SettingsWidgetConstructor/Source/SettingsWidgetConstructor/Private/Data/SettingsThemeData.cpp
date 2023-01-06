// Copyright (c) Yevhenii Selivanov

#include "Data/SettingsThemeData.h"

// Default constructor to set default values
FButtonThemeData::FButtonThemeData()
{
	Size = FVector2D(128.f);
	Margin = 0.25f;
}

// Default constructor to set default values
FCheckboxThemeData::FCheckboxThemeData()
{
	Size = FVector2D(24.f);
	DrawAs = ESlateBrushDrawType::Image;
}

// Default constructor to set default values
FComboboxThemeData::FComboboxThemeData()
{
	PressedPadding = FMargin(4.f, 6.5f, 2.f, 9.f);

	Arrow.Size = FVector2D(32.f);
	Arrow.DrawAs = ESlateBrushDrawType::Image;

	Border.Size = FVector2D(128.f);
	Border.Margin = 0.45f;
	Border.Padding = 5.f;

	Size = FVector2D(128.f);
	Margin = 0.25f;
	Padding = FMargin(4.f, 6.5f, 2.f, 9.f);
}

// Default constructor to set default values
FSliderThemeData::FSliderThemeData()
{
	Thumb.Size = FVector2D(12.f, 24.f);
	Thumb.DrawAs = ESlateBrushDrawType::Image;

	Size = FVector2D(128.f, 32.f);
}

// Default constructor to set default values
FMiscThemeData::FMiscThemeData()
{
	static const FLinearColor BlackOlive(FVector(0.046665f));
	ThemeColorNormal = BlackOlive;

	static const FLinearColor Nickel(FVector(0.168269f));
	ThemeColorHover = Nickel;
	ThemeColorExtra = Nickel;

	TooltipBackground.Size = FVector2D(128.f, 64.f);
	TooltipBackground.Margin = 0.2f;

	WindowBackground.Size = FVector2D(128.f, 64.f);
	WindowBackground.Margin = 0.3f;

	MenuBorderData.Margin = 0.45f;
}
