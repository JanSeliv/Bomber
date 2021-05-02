// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
//---
#include "Bomber.h"
//---
#include "SettingsRow.generated.h"

#define TEXT_NONE FCoreTexts::Get().None

/* ╔FSettingsRow
 * ╚════╦FSettingsPicker
 *		╠═══╦FSettingsPrimary
 *		║	╚════FSettingsFunction
 *		╚════FSettingsDataBase */

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

//ENUM_RANGE_BY_FIRST_AND_LAST($ENUM$, $ENUM$::First, $ENUM$::Last);
/**
  * Function wrapper
  */
USTRUCT(BlueprintType)
struct FSettingsFunction
{
	GENERATED_BODY()

	/** Empty settings function. */
	static const FSettingsFunction Empty;

	/**  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Class"))
	TSubclassOf<UObject> FunctionClass = nullptr; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Function"))
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
	float PaddingLeft = 0.f; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	float PaddingTop = 0.f; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	float PaddingRight = 0.f; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	float PaddingBottom = 0.f; //[D]

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
};

/**
 *
 */
USTRUCT(BlueprintType, meta = (
	SettingsFunctionSetterTemplate="OnSetterText__DelegateSignature",
	SettingsFunctionGetterTemplate="OnGetterText__DelegateSignature"))
struct FSettingsTextSimple : public FSettingsDataBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill; //[D]
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsTextInput : public FSettingsTextSimple
{
	GENERATED_BODY()
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
	FSettingsTextSimple TextSimple; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsTextInput TextInput; //[D]

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
 */
USTRUCT(BlueprintType)
struct FSettingsRow : public FTableRowBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	FSettingsPicker SettingsPicker = FSettingsPicker::Empty; //[D]
};
