// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/SlateWrapperTypes.h"
//---
#include "SettingsThemeData.generated.h"

/*	--- Settings Theme Data dependencies ---
*
*	╔USettingsDataAsset
*	╠════FButtonThemeData
*	╠════FCheckboxThemeData
*	╠════FComboboxThemeData
*	╠════FSliderThemeData
*	╚════FMiscThemeData
*/

/**
  * The parent struct of the settings theme data.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsThemeData
{
	GENERATED_BODY()

	/** The texture image of the setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UTexture> Texture = nullptr;

	/** The size of the resource in Slate Units. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector2D Size = FVector2D(64.f, 64.f);

	/** How to draw the image. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TEnumAsByte<ESlateBrushDrawType::Type> DrawAs = ESlateBrushDrawType::Box;

	/** The margin to use in Box and Border modes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMargin Margin;

	/** Outside padding of the image. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMargin Padding;
};

/**
  * The theme data of the settings button.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FButtonThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FButtonThemeData();

	/** The padding to used when button is pressed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMargin PressedPadding;
};

/**
  * The theme data of the settings checkbox.
  * The parent Texture property determines of the unchecked checkbox.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FCheckboxThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FCheckboxThemeData();

	/** The texture image of the toggled checkbox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UTexture> CheckedTexture = nullptr;

	/** The texture image of the undetermined checkbox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UTexture> UndeterminedTexture = nullptr;
};

/**
  * The theme data of the settings combobox.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FComboboxThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FComboboxThemeData();

	/** The padding to used when combobox is pressed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMargin PressedPadding;

	/** The combobox arrow theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSettingsThemeData Arrow;

	/** The combobox border theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSettingsThemeData Border;

	/** The combobox background color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor ItemBackgroundColor = FColor::Transparent;
};

/**
  * The theme data of the settings slider.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSliderThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FSliderThemeData();

	/** The theme of the slider thumb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSettingsThemeData Thumb;
};

/**
  * The common theme data.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FMiscThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FMiscThemeData();

	/** The common color of normal state for all setting types. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor ThemeColorNormal = FColor::White;

	/** The common color of hover state for all setting types. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor ThemeColorHover = FColor::White;

	/** The misc colors for all setting types. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor ThemeColorExtra = FColor::White;

	/** The font of text and captions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateFontInfo TextAndCaptionFont;

	/** The color of text and captions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor TextAndCaptionColor = FColor::White;

	/** The font of the header. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateFontInfo TextHeaderFont;

	/** The color of the header. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor TextHeaderColor = FColor::White;

	/** The font of the footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateFontInfo TextFooterFont;

	/** The color of the footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor TextFooterColor = FColor::White;

	/** The font of all setting values. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateFontInfo TextElementFont;

	/** The color of all setting values. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor TextElementColor = FColor::White;

	/** The theme data of tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSettingsThemeData TooltipBackground;

	/** The background color of tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor TooltipBackgroundTint = FColor::White;

	/** The theme data of the window background. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSettingsThemeData WindowBackground;

	/** The theme color of the window background. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor WindowBackgroundTint = FColor::White;

	/** The theme data of the menu border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSettingsThemeData MenuBorderData;

	/** Color of the border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FSlateColor MenuBorderTint = FColor::White;

	/** Visibility of the border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	ESlateVisibility MenuBorderVisibility = ESlateVisibility::Visible;
};
