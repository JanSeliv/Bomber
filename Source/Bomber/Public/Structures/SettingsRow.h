// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
//---
#include "Bomber.h"
//---
#include "SettingsRow.generated.h"

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Class"))
	TSubclassOf<UObject> FunctionClass = nullptr; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Function"))
	FName FunctionName = NAME_None; //[AW]

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
USTRUCT(BlueprintType)
struct FSettingsButton : public FSettingsDataBase
{
	GENERATED_BODY()
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsButtonsRow : public FSettingsDataBase
{
	GENERATED_BODY()
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	bool bIsSet = false; //[AW]
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 ChosenMemberIndex = INDEX_NONE; //[AW]
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

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float ChosenValue = static_cast<float>(INDEX_NONE); //[AW]
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FText CurrentText = FCoreTexts::Get().None; //[AW]
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FName SettingsType = NAME_None; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FGameplayTag Tag = FGameplayTag::EmptyTag; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (SettingsFunctionContextTemplate))
	FSettingsFunction StaticContext = FSettingsFunction::Empty; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (SettingsFunctionSetterTemplate))
	FSettingsFunction Setter = FSettingsFunction::Empty; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (SettingsFunctionGetterTemplate))
	FSettingsFunction Getter = FSettingsFunction::Empty; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FSettingsButton Button; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FSettingsButtonsRow ButtonsRow; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsCheckbox Checkbox; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsCombobox Combobox; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsSlider Slider; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsTextSimple TextSimple; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsTextInput TextInput; //[AW]

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsPicker SettingsPicker = FSettingsPicker::Empty; //[D]
};
