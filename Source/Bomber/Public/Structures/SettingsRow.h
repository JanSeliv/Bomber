// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
//---
#include "Bomber.h"
//---
#include "SettingsRow.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsFunction
{
	GENERATED_BODY()

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Class"))
	TSubclassOf<UObject> FunctionClass; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Function"))
	FName FunctionName; //[AW]

	/** Compares for equality.
	* @param Other The other object being compared. */
	bool operator==(const FSettingsFunction& Other) const;

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FSettingsFunction& Other);
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsDataBase
{
	GENERATED_BODY()

public:
	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FGameplayTag Tag; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsFunction ObjectContext; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsFunction Setter; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsFunction Getter; //[AW]

	/** Compares for equality.
	* @param Other The other object being compared. */
	bool operator==(const FSettingsDataBase& Other) const;

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FSettingsDataBase& Other);
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsButton : public FSettingsDataBase
{
	GENERATED_BODY()

public:
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsButtonsRow : public FSettingsDataBase
{
	GENERATED_BODY()

public:
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsCheckbox : public FSettingsDataBase
{
	GENERATED_BODY()

public:
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsCombobox : public FSettingsDataBase
{
	GENERATED_BODY()

public:
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsSlider : public FSettingsDataBase
{
	GENERATED_BODY()

public:
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsTextSimple : public FSettingsDataBase
{
	GENERATED_BODY()

public:
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsTextInput : public FSettingsTextSimple
{
	GENERATED_BODY()

public:
};

/**
*
*/
USTRUCT(BlueprintType)
struct FSettingsRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Empty settings row. */
	static const FSettingsRow EmptyRow;

	/** Contains a in-game settings type to be used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESettingsType SettingsType = ESettingsType::None; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsButton Button; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsButtonsRow ButtonsRow; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsCheckbox Checkbox; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsCombobox Combobox; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsSlider Slider; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsTextSimple TextSimple; //[AW]

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsTextInput TextInput; //[AW]

	/** */
	const FSettingsDataBase* GetChosenSettingsData() const;

	/** Returns true if row is valid. */
	FORCEINLINE bool IsValid() const { return !(*this == EmptyRow); }

	/** Compares for equality.
	 * @param Other The other object being compared. */
	bool operator==(const FSettingsRow& Other) const;

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FSettingsRow& Other);
};
