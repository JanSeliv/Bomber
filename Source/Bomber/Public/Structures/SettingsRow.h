// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
//---
#include "Bomber.h"
//---
#include "SettingsRow.generated.h"

#define TEXT_NONE FCoreTexts::Get().None

/**
 *
 */
UENUM(BlueprintType)
enum class EMyVerticalAlignment : uint8
{
	Header,
	Content,
	Footer
};

/**
 *
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
 *
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
 *
 */
UENUM(BlueprintType)
enum class ESettingsSliderState : uint8
{
	NormalBar,
	DisabledBar,
	NormalThumb,
	DisabledThumb
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsThemeData
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	class UTexture* Texture; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FVector2D Size = FVector2D(64.f, 64.f); //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	TEnumAsByte<enum ESlateBrushDrawType::Type> DrawAs = ESlateBrushDrawType::Box; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FMargin Margin; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FMargin Padding; //[B]
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FButtonThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FMargin PressedPadding; //[B]
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FCheckboxThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	class UTexture* CheckedTexture; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	class UTexture* UndeterminedTexture; //[B]
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FComboboxThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FMargin PressedPadding; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSettingsThemeData Arrow; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSettingsThemeData Border; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor ItemBackgroundColor; //[B]
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSliderThemeData : public FSettingsThemeData
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor BarNormalTint;

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor BarDisabledTint;

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSettingsThemeData Thumb;

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor ThumbNormalTint;

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor ThumbDisabledTint;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FMiscThemeData
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor ThemeColorNormal; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor ThemeColorHover; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor ThemeColorExtra; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateFontInfo TextAndCaptionFont; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor TextAndCaptionColor; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateFontInfo TextHeaderFont; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor TextHeaderColor; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateFontInfo TextFooterFont; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor TextFooterColor; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateFontInfo TextElementFont; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor TextElementColor; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateFontInfo BottomLineFont; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor BottomLineColor; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSettingsThemeData TooltipBackground; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor TooltipBackgroundTint; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSettingsThemeData WindowBackground; //[B]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Theme")
	FSlateColor WindowBackgroundTint; //[B]
};

/* ╔FSettingsRow
 * ╚════╦FSettingsPicker
 *		╠═══╦FSettingsPrimary
 *		║	╚════FSettingsFunction
 *		╚════FSettingsDataBase */

/**
* Delegates wrapper that are used as templates for FSettingsFunction properties.
* Has to have reflection to allow find its members by FSettingsFunctionCustomization:
* USettingTemplate::StaticClass()->FindFunctionByName("OnStaticContext__DelegateSignature");
* DECLARE_DYNAMIC_DELEGATE can't be declared under USTRUCT
*/
UCLASS(Abstract, Const, Transient)
class BOMBER_API USettingTemplate final : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(UObject*, FOnStaticContext);

	DECLARE_DYNAMIC_DELEGATE(FOnButtonPressed);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterInt, int32, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterFloat, float, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterBool, bool, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterText, FText, Param);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetMembers, const TArray<FText>&, NewMembers);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetterName, FName, Param);

	DECLARE_DYNAMIC_DELEGATE_RetVal(int32, FOnGetterInt);

	DECLARE_DYNAMIC_DELEGATE_RetVal(float, FOnGetterFloat);

	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FOnGetterBool);

	DECLARE_DYNAMIC_DELEGATE_RetVal(FText, FOnGetterText);

	DECLARE_DYNAMIC_DELEGATE_RetVal(TArray<FText>, FOnGetMembers);

	DECLARE_DYNAMIC_DELEGATE_RetVal(FName, FOnGetterName);
};

/**
  * Function wrapper
  */
USTRUCT(BlueprintType)
struct FSettingsFunction
{
	GENERATED_BODY()

	/** Empty settings function. */
	static const FSettingsFunction Empty;

	/** */
	FSettingsFunction() = default;

	/** */
	FSettingsFunction(TSubclassOf<UObject> InFunctionClass, FName InFunctionName);

	/**  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (DisplayName = "Class"))
	TSubclassOf<UObject> FunctionClass = nullptr; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (DisplayName = "Function"))
	FName FunctionName = NAME_None; //[D]

	/** Returns true if is valid. */
	FORCEINLINE bool IsValid() const { return !(*this == Empty); }

	/** Compares for equality.
	* @param Other The other object being compared. */
	bool operator==(const FSettingsFunction& Other) const;

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FSettingsFunction& Other);
};

/**
* The primary data of any setting.
*/
USTRUCT(BlueprintType)
struct FSettingsPrimary
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FGameplayTag Tag = FGameplayTag::EmptyTag; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (SettingsFunctionContextTemplate))
	FSettingsFunction StaticContext = FSettingsFunction::Empty; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (SettingsFunctionSetterTemplate))
	FSettingsFunction Setter = FSettingsFunction::Empty; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (SettingsFunctionGetterTemplate))
	FSettingsFunction Getter = FSettingsFunction::Empty; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FText Caption = TEXT_NONE; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FText Tooltip = TEXT_NONE; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FMargin Padding = 0.f;

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	bool bStartOnNextColumn; //[D]

	/** */
	TWeakObjectPtr<UObject> StaticContextObject;

	/** */
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
	SettingsFunctionContextTemplate="OnStaticContext__DelegateSignature"))
struct FSettingsDataBase
{
	GENERATED_BODY()
};

/**
 *
 */
USTRUCT(BlueprintType, meta = (
	SettingsFunctionSetterTemplate="OnButtonPressed__DelegateSignature"))
struct FSettingsButton : public FSettingsDataBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	float LineHeight = 0.f; //[D]

	/** */
	USettingTemplate::FOnButtonPressed OnButtonPressed;
};

/**
 *
 */
USTRUCT(BlueprintType, meta = (
	SettingsFunctionSetterTemplate="OnSetterBool__DelegateSignature",
	SettingsFunctionGetterTemplate="OnGetterBool__DelegateSignature"))
struct FSettingsCheckbox : public FSettingsDataBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	bool bIsSet = false; //[D]

	/** */
	USettingTemplate::FOnGetterBool OnGetterBool;

	/** */
	USettingTemplate::FOnSetterBool OnSetterBool;
};

/**
 *
 */
USTRUCT(BlueprintType, meta = (
	SettingsFunctionSetterTemplate="OnSetterInt__DelegateSignature",
	SettingsFunctionGetterTemplate="OnGetterInt__DelegateSignature"))
struct FSettingsCombobox : public FSettingsDataBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (SettingsFunctionSetterTemplate="OnSetMembers__DelegateSignature"))
	FSettingsFunction SetMembers = FSettingsFunction::Empty; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (SettingsFunctionGetterTemplate="OnGetMembers__DelegateSignature"))
	FSettingsFunction GetMembers = FSettingsFunction::Empty; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	int32 ChosenMemberIndex = INDEX_NONE; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TArray<FText> Members; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TEnumAsByte<ETextJustify::Type> TextJustify = ETextJustify::Center; //[D]

	/** */
	USettingTemplate::FOnGetterInt OnGetterInt;

	/** */
	USettingTemplate::FOnSetterInt OnSetterInt;

	/** */
	USettingTemplate::FOnGetMembers OnGetMembers;

	/** */
	USettingTemplate::FOnSetMembers OnSetMembers;
};

/**
 *
 */
USTRUCT(BlueprintType, meta = (
	SettingsFunctionSetterTemplate="OnSetterFloat__DelegateSignature",
	SettingsFunctionGetterTemplate="OnGetterFloat__DelegateSignature"))
struct FSettingsSlider : public FSettingsDataBase
{
	GENERATED_BODY()

	/** Slider value (0..1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChosenValue = 0.5f; //[D]

	/** */
	USettingTemplate::FOnGetterFloat OnGetterFloat;

	/** */
	USettingTemplate::FOnSetterFloat OnSetterFloat;
};

/**
 *
 */
USTRUCT(BlueprintType, meta = (
	SettingsFunctionSetterTemplate="OnSetterText__DelegateSignature",
	SettingsFunctionGetterTemplate="OnGetterText__DelegateSignature"))
struct FSettingsTextLine : public FSettingsDataBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill; //[D]

	/** */
	USettingTemplate::FOnGetterText OnGetterText;

	/** */
	USettingTemplate::FOnSetterText OnSetterText;
};

/**
 *
 */
USTRUCT(BlueprintType, meta = (
	SettingsFunctionSetterTemplate="OnSetterName__DelegateSignature",
	SettingsFunctionGetterTemplate="OnGetterName__DelegateSignature"))
struct FSettingsUserInput : public FSettingsDataBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FName UserInput = NAME_None; //[D]

	/** */
	USettingTemplate::FOnGetterName OnGetterName;

	/** */
	USettingTemplate::FOnSetterName OnSetterName;
};

/**
 * is customizable struct, its members were created under FSettingsPicker ...
 * (instead of FSettingsRow which implements table row struct and can't be customized),
 * to have possibility to be property-customized by FSettingsPickerCustomization,
 * which allows to show only selected in-game option.
 */
USTRUCT(BlueprintType)
struct FSettingsPicker
{
	GENERATED_BODY()

	/** Nothing picked. */
	static const FSettingsPicker Empty;

	/** Contains a in-game settings type to be used. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FName SettingsType = NAME_None; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsPrimary PrimaryData;

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsButton Button; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsCheckbox Checkbox; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsCombobox Combobox; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsSlider Slider; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsTextLine TextLine; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsUserInput UserInput; //[D]

	/** Returns the pointer to one of the chosen in-game type.
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
struct FSettingsRow : public FTableRowBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsPicker SettingsPicker = FSettingsPicker::Empty; //[D]
};
