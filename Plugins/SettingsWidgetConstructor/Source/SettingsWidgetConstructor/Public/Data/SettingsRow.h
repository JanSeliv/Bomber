// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "MyDataTable/MyDataTable.h"
#include "SettingsWidgetConstructor/Private/FunctionPickerData/SWCFunctionPicker.h"
#include "SettingsWidgetConstructor/Private/FunctionPickerData/SWCFunctionPickerTemplate.h"
//---
#include "GameplayTagContainer.h"
#include "Components/SlateWrapperTypes.h"
//---
#include "SettingsRow.generated.h"

#define TEXT_NONE FCoreTexts::Get().None

/* --- Settings Data Table dependencies ---
 *
 *	╔USettingsDataTable
 *	╚═══╦FSettingsRow
 *		╚═══╦FSettingsPicker
 *			╠═══╦FSettingsPrimary
 *			║	╠════FSettingTag
 *			║	╚════FSettingFunctionPicker (StaticContext and Setter and Getter)
 *			╚════FSettingsDataBase (FSettingsButton or FSettingsCheckbox or
 *									FSettingsCombobox or FSettingsCustomWidget or
 *									FSettingsSlider or FSettingsTextLine, FSettingsUserInput)
 *
 *	--- Settings Theme Data dependencies ---
 *
 *	╔USettingsDataAsset
 *	╠════FButtonThemeData
 *	╠════FCheckboxThemeData
 *	╠════FComboboxThemeData
 *	╠════FSliderThemeData
 *	╚════FMiscThemeData
 */

/**
 * Used to require all such tags start with 'Settings.X' as it specified in USTRUCT meta.
 */
USTRUCT(BlueprintType, meta = (Categories = "Settings"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingTag : public FGameplayTag
{
	GENERATED_BODY()

	FSettingTag() = default;

	FSettingTag(const FGameplayTag& Tag)
		: FGameplayTag(Tag) {}

	static const FSettingTag EmptySettingTag;
};

/**
 * Allows automatically add native setting tags at startup.
 */
struct SETTINGSWIDGETCONSTRUCTOR_API FGlobalSettingTags : public FGameplayTagNativeAdder
{
	FSettingTag ButtonSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag CheckboxSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag ComboboxSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag ScrollboxSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag SliderSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag TextLineSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag UserInputSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag CustomWidgetSettingTag = FSettingTag::EmptySettingTag;

	/** Returns global setting tags as const ref.
	 * @see FGlobalSettingTags::GSettingTags. */
	static const FORCEINLINE FGlobalSettingTags& Get() { return GSettingTags; }

	virtual void AddTags() override;
	virtual ~FGlobalSettingTags() = default;

private:
	/** The global of Setting tag categories. */
	static FGlobalSettingTags GSettingTags;
};

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
	TEnumAsByte<enum ESlateBrushDrawType::Type> DrawAs = ESlateBrushDrawType::Box;

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

/**
 * Is used to select setting function to be called
 */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingFunctionPicker : public FSWCFunctionPicker
{
	GENERATED_BODY()

	/** Empty setting function data. */
	static const FSettingFunctionPicker EmptySettingFunction;

	/** Default constructor. */
	FSettingFunctionPicker() = default;

	/** Custom constructor to set all members values. */
	FSettingFunctionPicker(UClass* InFunctionClass, FName InFunctionName)
		: FSWCFunctionPicker(InFunctionClass, std::move(InFunctionName)) {}
};

/**
  * Delegates wrapper that are used as templates for FSettingFunctionPicker properties.
  * see UFunctionPickerTemplate
  */
UCLASS(Abstract, Const, Transient)
class SETTINGSWIDGETCONSTRUCTOR_API USettingFunctionTemplate : public USWCFunctionPickerTemplate
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterWidget, class USettingCustomWidget*, Param);

	DECLARE_DYNAMIC_DELEGATE_RetVal(class USettingCustomWidget*, FOnGetterWidget);
};

/**
  * The primary data of any setting.
  * Does not contain a default states for its value, because it should be set in the DefaultGameUserSettings.ini
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsPrimary
{
	GENERATED_BODY()

	/** Empty settings primary row. */
	static const FSettingsPrimary EmptyPrimary;

	/** The tag of the setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingTag Tag = FSettingTag::EmptySettingTag;

	/** The static function to obtain object to call Setters and Getters.
	  * The FunctionContextTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (FunctionContextTemplate))
	FSettingFunctionPicker StaticContext = FSettingFunctionPicker::EmptySettingFunction;

	/** The Setter function to be called to set the setting value for the Static Context object.
	  * The FunctionSetterTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (FunctionSetterTemplate))
	FSettingFunctionPicker Setter = FSettingFunctionPicker::EmptySettingFunction;

	/** The Getter function to be called to get the setting value from the Static Context object.
	  * The FunctionGetterTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (FunctionGetterTemplate))
	FSettingFunctionPicker Getter = FSettingFunctionPicker::EmptySettingFunction;

	/** The setting name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Caption = TEXT_NONE;

	/** The description to be shown as tooltip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Tooltip = TEXT_NONE;

	/** The padding of this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMargin Padding = 0.f;

	/** The custom line height for this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float LineHeight = 48.f;

	/** Set true to add new column starting from this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bStartOnNextColumn = false;

	/** Contains tags of settings which are needed to update after change of this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Settings"))
	FGameplayTagContainer SettingsToUpdate = FGameplayTagContainer::EmptyContainer;

	/** Created widget of the chosen setting (button, checkbox, combobox, slider, text line, user input). */
	TWeakObjectPtr<class USettingSubWidget> SettingSubWidget = nullptr;

	/** The cached object obtained from the Static Context function. */
	TWeakObjectPtr<UObject> StaticContextObject = nullptr;

	/** Contains all cached functions of the Static Context object. */
	TArray<FName> StaticContextFunctionList;

	/** Returns true if is valid. */
	FORCEINLINE bool IsValid() const { return Tag.IsValid(); }

	/** Compares for equality.
	* @param Other The other object being compared. */
	bool operator==(const FSettingsPrimary& Other) const;

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FSettingsPrimary& Other);
};

/**
  * The base archetype of some setting.
  */
USTRUCT(BlueprintType, meta = (
	FunctionContextTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnStaticContext__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsDataBase
{
	GENERATED_BODY()
};

/**
  * The setting button data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnButtonPressed__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsButton : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Either Header, Content, or Footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content;

	/** Either Left, Right, Center, or Fill. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;

	/** Cached bound delegate, is executed on pressing this button. */
	USettingFunctionTemplate::FOnButtonPressed OnButtonPressed;
};

/**
  * The setting checkbox data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterBool__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterBool__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsCheckbox : public FSettingsDataBase
{
	GENERATED_BODY()

	/** The cached current checkbox state. */
	bool bIsSet = false;

	/** The cached bound delegate, is executed to get the current checkbox state. */
	USettingFunctionTemplate::FOnGetterBool OnGetterBool;

	/** The cached bound delegate, is executed to set the current checkbox state. */
	USettingFunctionTemplate::FOnSetterBool OnSetterBool;
};

/**
  * The setting combobox data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterInt__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterInt__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsCombobox : public FSettingsDataBase
{
	GENERATED_BODY()

	/** The Setter function to be called to set all combobox members. */
	UPROPERTY(EditDefaultsOnly, meta = (FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetMembers__DelegateSignature"))
	FSettingFunctionPicker SetMembers = FSettingFunctionPicker::EmptySettingFunction;

	/** The Setter function to be called to get all combobox members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetMembers__DelegateSignature"))
	FSettingFunctionPicker GetMembers = FSettingFunctionPicker::EmptySettingFunction;

	/** Contains all combobox members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FText> Members;

	/** Text alignment either left, center, or right. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<ETextJustify::Type> TextJustify = ETextJustify::Center;

	/** The cached chosen member index. */
	int32 ChosenMemberIndex = INDEX_NONE;

	/** The cached bound delegate, is executed to get the chosen member index. */
	USettingFunctionTemplate::FOnGetterInt OnGetterInt;

	/** The cached bound delegate, is executed to set the chosen member index. */
	USettingFunctionTemplate::FOnSetterInt OnSetterInt;

	/** The cached bound delegate, is executed to get all combobox members. */
	USettingFunctionTemplate::FOnGetMembers OnGetMembers;

	/** The cached bound delegate, is executed to set all combobox members. */
	USettingFunctionTemplate::FOnSetMembers OnSetMembers;
};

/**
  * The setting slider data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterFloat__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterFloat__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsSlider : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Cached slider value (0..1). */
	double ChosenValue = INDEX_NONE;

	/** The cached bound delegate, is executed to get the current slider value. */
	USettingFunctionTemplate::FOnGetterFloat OnGetterFloat;

	/** The cached bound delegate, is executed to set the current slider value. */
	USettingFunctionTemplate::FOnSetterFloat OnSetterFloat;
};

/**
  * The setting text line data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterText__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterText__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsTextLine : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Either Header, Content, or Footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content;

	/** Either Left, Right, Center, or Fill. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;

	/** The cached bound delegate, is executed to set the text caption. */
	USettingFunctionTemplate::FOnGetterText OnGetterText;

	/** The cached bound delegate, is executed to get the text caption. */
	USettingFunctionTemplate::FOnSetterText OnSetterText;
};

/**
  * The setting user input data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterName__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterName__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsUserInput : public FSettingsDataBase
{
	GENERATED_BODY()

	/** The maximal length of the player input that is allowed to type.
	 * Set 0 to do not limit number of characters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxCharactersNumber = 0;

	/** The cached text shown left of the input box. */
	FName UserInput = NAME_None;

	/** The cached bound delegate, is executed to set the input text. */
	USettingFunctionTemplate::FOnGetterName OnGetterName;

	/** The cached bound delegate, is executed to get the input text. */
	USettingFunctionTemplate::FOnSetterName OnSetterName;
};

/**
  * The setting user input data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterWidget__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterWidget__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsCustomWidget : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Contains created custom widget of the setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TSubclassOf<class USettingCustomWidget> CustomWidgetClass = nullptr;

	/** The cached bound delegate, is executed to set the custom widget. */
	USettingFunctionTemplate::FOnGetterWidget OnGetterWidget;

	/** The cached bound delegate, is executed to get the custom widget. */
	USettingFunctionTemplate::FOnSetterWidget OnSetterWidget;
};

/**
  * Is customizable struct, its members were created under FSettingsPicker ...
  * (instead of FSettingsRow which implements table row struct and can't be customized),
  * to have possibility to be property-customized by FSettingsPickerCustomization,
  * which allows to show only selected in-game option.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsPicker
{
	GENERATED_BODY()

	/** Nothing picked. */
	static const FSettingsPicker Empty;

	/** Contains a in-game settings type to be used - the name of one of these members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SettingsType = NAME_None;

	/** The common setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsPrimary PrimaryData;

	/** The button setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsButton Button;

	/** The checkbox setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsCheckbox Checkbox;

	/** The combobox setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsCombobox Combobox;

	/** The slider setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsSlider Slider;

	/** The text line setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsTextLine TextLine;

	/** The user input setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsUserInput UserInput;

	/** The custom widget setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsCustomWidget CustomWidget;

	/** Returns the pointer to one of the chosen in-game type.
	  * It searches the member property of this struct by a value of SettingsType.
	  * @see FSettingsPicker::SettingsType */
	const FSettingsDataBase* GetChosenSettingsData() const;

	/** Returns true if row is valid. */
	FORCEINLINE bool IsValid() const { return !(*this == Empty); }

	/** Compares for equality.
	  * @param Other The other object being compared. */
	bool operator==(const FSettingsPicker& Other) const;

	/** Creates a hash value.
	  * @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FSettingsPicker& Other);
};

/**
  * Row of the settings data table.
  * In a row can be specified all UI values, chosen any getter/setter in the list.
  * Main features:
  * By this row, new setting will be automatically added on UI.
  * Executing UI getters/setters will call automatically bounded chosen functions.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsRow : public FMyTableRow
{
	GENERATED_BODY()

	/** The setting row to be customised. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsPicker SettingsPicker = FSettingsPicker::Empty;
};
