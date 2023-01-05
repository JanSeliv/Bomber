// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "Data/SettingsRow.h"
//---
#include "SettingsWidgetConstructorLibrary.generated.h"

/**
 * The settings widget constructor functions library
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingsWidgetConstructorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if row is valid. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library", meta = (AutoCreateRefTerm = "SettingsRow"))
	static FORCEINLINE bool IsValidSettingsRow(const FSettingsPicker& SettingsRow) { return SettingsRow.IsValid(); }

	/** Returns empty settings row. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingsPicker& GetEmptySettingsRow() { return FSettingsPicker::Empty; }

	/** Returns empty setting tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetEmptySettingTag() { return FSettingTag::EmptySettingTag; }

	/** Returns the 'Settings.Button' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetButtonSettingTag() { return FGlobalSettingTags::Get().ButtonSettingTag; }

	/** Returns the 'Settings.Checkbox' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetCheckboxSettingTag() { return FGlobalSettingTags::Get().CheckboxSettingTag; }

	/** Returns the 'Settings.Combobox' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetComboboxSettingTag() { return FGlobalSettingTags::Get().ComboboxSettingTag; }

	/** Returns the 'Settings.Scrollbox' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetScrollboxSettingTag() { return FGlobalSettingTags::Get().ScrollboxSettingTag; }

	/** Returns the 'Settings.Slider' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetSliderSettingTag() { return FGlobalSettingTags::Get().SliderSettingTag; }

	/** Returns the 'Settings.TextLine' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetTextLineSettingTag() { return FGlobalSettingTags::Get().TextLineSettingTag; }

	/** Returns the 'Settings.UserInput' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetUserInputSettingTag() { return FGlobalSettingTags::Get().UserInputSettingTag; }

	/** Returns the 'Settings.CustomWidget' tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Static Library")
	static const FORCEINLINE FSettingTag& GetCustomWidgetSettingTag() { return FGlobalSettingTags::Get().CustomWidgetSettingTag; }
};
