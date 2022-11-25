// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "MyDataTable/MyDataTable.h"
#include "FunctionPicker/FunctionPicker.h"
#include "FunctionPicker/FunctionPickerTemplate.h"
//---
#include "GameplayTagContainer.h"
#include "Components/SlateWrapperTypes.h"
//---
#include "SettingsRow.generated.h"

#define TEXT_NONE FCoreTexts::Get().None

/**
 * Used to require all such tags start with 'Settings.X' as it specified in USTRUCT meta.
 */
USTRUCT(BlueprintType, meta = (Categories = "Settings"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingTag : public FGameplayTag
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
struct MYSETTINGSWIDGETCONSTRUCTOR_API FGlobalSettingTags : public FGameplayTagNativeAdder
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
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsThemeData
{
	GENERATED_BODY()

	/** The texture image of the setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<class UTexture> Texture = nullptr; //[B]

	/** The size of the resource in Slate Units. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FVector2D Size = FVector2D(64.f, 64.f); //[B]

	/** How to draw the image. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TEnumAsByte<enum ESlateBrushDrawType::Type> DrawAs = ESlateBrushDrawType::Box; //[B]

	/** The margin to use in Box and Border modes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FMargin Margin; //[B]

	/** Outside padding of the image. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FMargin Padding; //[B]
};

/**
  * The theme data of the settings button.
  */
USTRUCT(BlueprintType)
struct MYSETTINGSWIDGETCONSTRUCTOR_API FButtonThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FButtonThemeData();

	/** The padding to used when button is pressed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FMargin PressedPadding; //[B]
};

/**
  * The theme data of the settings checkbox.
  * The parent Texture property determines of the unchecked checkbox.
  */
USTRUCT(BlueprintType)
struct MYSETTINGSWIDGETCONSTRUCTOR_API FCheckboxThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FCheckboxThemeData();

	/** The texture image of the toggled checkbox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<class UTexture> CheckedTexture = nullptr; //[B]

	/** The texture image of the undetermined checkbox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<class UTexture> UndeterminedTexture = nullptr; //[B]
};

/**
  * The theme data of the settings combobox.
  */
USTRUCT(BlueprintType)
struct MYSETTINGSWIDGETCONSTRUCTOR_API FComboboxThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FComboboxThemeData();

	/** The padding to used when combobox is pressed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FMargin PressedPadding; //[B]

	/** The combobox arrow theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSettingsThemeData Arrow; //[B]

	/** The combobox border theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSettingsThemeData Border; //[B]

	/** The combobox background color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor ItemBackgroundColor = FColor::Transparent; //[B]
};

/**
  * The theme data of the settings slider.
  */
USTRUCT(BlueprintType)
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSliderThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FSliderThemeData();

	/** The theme of the slider thumb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSettingsThemeData Thumb;
};

/**
  * The common theme data.
  */
USTRUCT(BlueprintType)
struct MYSETTINGSWIDGETCONSTRUCTOR_API FMiscThemeData
{
	GENERATED_BODY()

	/** Default constructor to set default values. */
	FMiscThemeData();

	/** The common color of normal state for all setting types. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor ThemeColorNormal = FColor::White; //[B]

	/** The common color of hover state for all setting types. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor ThemeColorHover = FColor::White; //[B]

	/** The misc colors for all setting types. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor ThemeColorExtra = FColor::White; //[B]

	/** The font of text and captions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateFontInfo TextAndCaptionFont; //[B]

	/** The color of text and captions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor TextAndCaptionColor = FColor::White; //[B]

	/** The font of the header. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateFontInfo TextHeaderFont; //[B]

	/** The color of the header. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor TextHeaderColor = FColor::White; //[B]

	/** The font of the footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateFontInfo TextFooterFont; //[B]

	/** The color of the footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor TextFooterColor = FColor::White; //[B]

	/** The font of all setting values. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateFontInfo TextElementFont; //[B]

	/** The color of all setting values. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor TextElementColor = FColor::White; //[B]

	/** The theme data of tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSettingsThemeData TooltipBackground; //[B]

	/** The background color of tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor TooltipBackgroundTint = FColor::White; //[B]

	/** The theme data of the window background. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSettingsThemeData WindowBackground; //[B]

	/** The theme color of the window background. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor WindowBackgroundTint = FColor::White; //[B]

	/** The theme data of the menu border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSettingsThemeData MenuBorderData; //[B]

	/** Color of the border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FSlateColor MenuBorderTint = FColor::White; //[B]

	/** Visibility of the border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	ESlateVisibility MenuBorderVisibility = ESlateVisibility::Visible; //[B]
};

/* Structs dependencies:
 *
 * ╔FSettingsRow
 * ╚════╦FSettingsPicker
 *		╠════FSettingsPrimary
 *		╚════FSettingsDataBase
 */

/**
  * Delegates wrapper that are used as templates for FFunctionPicker properties.
  * see UFunctionPickerTemplate
  */
UCLASS(Abstract, Const, Transient)
class MYSETTINGSWIDGETCONSTRUCTOR_API USettingTemplate : public UFunctionPickerTemplate
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
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsPrimary
{
	GENERATED_BODY()

	/** The tag of the setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingTag Tag = FSettingTag::EmptySettingTag; //[D]

	/** The static function to obtain object to call Setters and Getters.
	  * The FunctionContextTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (FunctionContextTemplate))
	FFunctionPicker StaticContext = FFunctionPicker::Empty; //[D]

	/** The Setter function to be called to set the setting value for the Static Context object.
	  * The FunctionSetterTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (FunctionSetterTemplate))
	FFunctionPicker Setter = FFunctionPicker::Empty; //[D]

	/** The Getter function to be called to get the setting value from the Static Context object.
	  * The FunctionGetterTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (FunctionGetterTemplate))
	FFunctionPicker Getter = FFunctionPicker::Empty; //[D]

	/** The setting name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FText Caption = TEXT_NONE; //[D]

	/** The description to be shown as tooltip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FText Tooltip = TEXT_NONE; //[D]

	/** The padding of this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FMargin Padding = 0.f; //[D]

	/** The custom line height for this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	float LineHeight = 48.f; //[D]

	/** Set true to add new column starting from this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	bool bStartOnNextColumn = false; //[D]

	/** Contains tags of settings which are needed to update after change of this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (Categories = "Settings"))
	FGameplayTagContainer SettingsToUpdate = FGameplayTagContainer::EmptyContainer; //[D]

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
	FunctionContextTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnStaticContext__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsDataBase
{
	GENERATED_BODY()
};

/**
  * The setting button data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnButtonPressed__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsButton : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Either Header, Content, or Footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content; //[D]

	/** Either Left, Right, Center, or Fill. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill; //[D]

	/** Cached bound delegate, is executed on pressing this button. */
	UFunctionPickerTemplate::FOnButtonPressed OnButtonPressed;
};

/**
  * The setting checkbox data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnSetterBool__DelegateSignature",
	FunctionGetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnGetterBool__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsCheckbox : public FSettingsDataBase
{
	GENERATED_BODY()

	/** The cached current checkbox state. */
	bool bIsSet = false;

	/** The cached bound delegate, is executed to get the current checkbox state. */
	UFunctionPickerTemplate::FOnGetterBool OnGetterBool;

	/** The cached bound delegate, is executed to set the current checkbox state. */
	UFunctionPickerTemplate::FOnSetterBool OnSetterBool;
};

/**
  * The setting combobox data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnSetterInt__DelegateSignature",
	FunctionGetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnGetterInt__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsCombobox : public FSettingsDataBase
{
	GENERATED_BODY()

	/** The Setter function to be called to set all combobox members. */
	UPROPERTY(EditDefaultsOnly, meta = (FunctionSetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnSetMembers__DelegateSignature"))
	FFunctionPicker SetMembers = FFunctionPicker::Empty; //[D]

	/** The Setter function to be called to get all combobox members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (FunctionGetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnGetMembers__DelegateSignature"))
	FFunctionPicker GetMembers = FFunctionPicker::Empty; //[D]

	/** Contains all combobox members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TArray<FText> Members; //[D]

	/** Text alignment either left, center, or right. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TEnumAsByte<ETextJustify::Type> TextJustify = ETextJustify::Center; //[D]

	/** The cached chosen member index. */
	int32 ChosenMemberIndex = INDEX_NONE;

	/** The cached bound delegate, is executed to get the chosen member index. */
	UFunctionPickerTemplate::FOnGetterInt OnGetterInt;

	/** The cached bound delegate, is executed to set the chosen member index. */
	UFunctionPickerTemplate::FOnSetterInt OnSetterInt;

	/** The cached bound delegate, is executed to get all combobox members. */
	UFunctionPickerTemplate::FOnGetMembers OnGetMembers;

	/** The cached bound delegate, is executed to set all combobox members. */
	UFunctionPickerTemplate::FOnSetMembers OnSetMembers;
};

/**
  * The setting slider data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnSetterFloat__DelegateSignature",
	FunctionGetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnGetterFloat__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsSlider : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Cached slider value (0..1). */
	double ChosenValue = INDEX_NONE;

	/** The cached bound delegate, is executed to get the current slider value. */
	UFunctionPickerTemplate::FOnGetterFloat OnGetterFloat;

	/** The cached bound delegate, is executed to set the current slider value. */
	UFunctionPickerTemplate::FOnSetterFloat OnSetterFloat;
};

/**
  * The setting text line data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnSetterText__DelegateSignature",
	FunctionGetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnGetterText__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsTextLine : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Either Header, Content, or Footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content; //[D]

	/** Either Left, Right, Center, or Fill. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill; //[D]

	/** The cached bound delegate, is executed to set the text caption. */
	UFunctionPickerTemplate::FOnGetterText OnGetterText;

	/** The cached bound delegate, is executed to get the text caption. */
	UFunctionPickerTemplate::FOnSetterText OnSetterText;
};

/**
  * The setting user input data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnSetterName__DelegateSignature",
	FunctionGetterTemplate = "/Script/MyUtils.FunctionPickerTemplate::OnGetterName__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsUserInput : public FSettingsDataBase
{
	GENERATED_BODY()

	/** The maximal length of the player input that is allowed to type.
	 * Set 0 to do not limit number of characters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	int32 MaxCharactersNumber = 0; //[D]

	/** The cached text shown left of the input box. */
	FName UserInput = NAME_None;

	/** The cached bound delegate, is executed to set the input text. */
	UFunctionPickerTemplate::FOnGetterName OnGetterName;

	/** The cached bound delegate, is executed to get the input text. */
	UFunctionPickerTemplate::FOnSetterName OnSetterName;
};

/**
  * The setting user input data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/MySettingsWidgetConstructor.SettingTemplate::OnSetterWidget__DelegateSignature",
	FunctionGetterTemplate = "/Script/MySettingsWidgetConstructor.SettingTemplate::OnGetterWidget__DelegateSignature"))
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsCustomWidget : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Contains created custom widget of the setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TSubclassOf<class USettingCustomWidget> CustomWidgetClass = nullptr; //[D]

	/** The cached bound delegate, is executed to set the custom widget. */
	USettingTemplate::FOnGetterWidget OnGetterWidget;

	/** The cached bound delegate, is executed to get the custom widget. */
	USettingTemplate::FOnSetterWidget OnSetterWidget;
};

/**
  * Is customizable struct, its members were created under FSettingsPicker ...
  * (instead of FSettingsRow which implements table row struct and can't be customized),
  * to have possibility to be property-customized by FSettingsPickerCustomization,
  * which allows to show only selected in-game option.
  */
USTRUCT(BlueprintType)
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsPicker
{
	GENERATED_BODY()

	/** Nothing picked. */
	static const FSettingsPicker Empty;

	/** Contains a in-game settings type to be used - the name of one of these members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FName SettingsType = NAME_None; //[D]

	/** The common setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsPrimary PrimaryData; //[D]

	/** The button setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsButton Button; //[D]

	/** The checkbox setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsCheckbox Checkbox; //[D]

	/** The combobox setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsCombobox Combobox; //[D]

	/** The slider setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsSlider Slider; //[D]

	/** The text line setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsTextLine TextLine; //[D]

	/** The user input setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsUserInput UserInput; //[D]

	/** The custom widget setting data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsCustomWidget CustomWidget; //[D]

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
struct MYSETTINGSWIDGETCONSTRUCTOR_API FSettingsRow : public FMyTableRow
{
	GENERATED_BODY()

	/** The setting row to be customised. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsPicker SettingsPicker = FSettingsPicker::Empty; //[D]
};
