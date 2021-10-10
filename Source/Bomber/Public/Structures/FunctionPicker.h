// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "FunctionPicker.generated.h"

/**
  * Allows designer to choose the function in the list.
  *
  * It's possible to set the template by specifying the function or delegate in the UPROPERTY's meta.
  * It will filter the list of all functions by its return type and type of all function arguments.
  *	If none meta is set, all functions will appear in the list without any filtering.
  *
  * Meta keys:
  * FunctionContextTemplate (only to show static functions)
  * FunctionGetterTemplate
  * FunctionSetterTemplate
  *
  * Meta value (without prefixes):
  * ClassName::FunctionName
  *
  * Example:
  * UPROPERTY(EditDefaultsOnly, meta = (FunctionSetterTemplate="SettingTemplate::OnSetMembers__DelegateSignature"))
  * FFunctionPicker SetMembers = FFunctionPicker::Empty;
  */
USTRUCT(BlueprintType)
struct FFunctionPicker
{
	GENERATED_BODY()

	/** Empty settings function. */
	static const FFunctionPicker Empty;

	/** Default constructor. */
	FFunctionPicker() = default;

	/** Custom constructor to set all members values. */
	FFunctionPicker(UClass* InFunctionClass, FName InFunctionName);

	/** The class where function can be found. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (DisplayName = "Class"))
	UClass* FunctionClass = nullptr; //[D]

	/** The function name to choose for specified class.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (DisplayName = "Function"))
	FName FunctionName = NAME_None; //[D]

	/** Returns true if is valid. */
	FORCEINLINE bool IsValid() const { return !(*this == Empty); }

	/** Returns the function pointer based on set data to this structure. */
	UFunction* GetFunction() const;

	/** Compares for equality.
	  * @param Other The other object being compared. */
	bool operator==(const FFunctionPicker& Other) const;

	/** Creates a hash value.
	  * @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FFunctionPicker& Other);

protected:
	/** Contains cached function ptr for performance reasons. */
	mutable TWeakObjectPtr<UFunction> CachedFunctionInternal = nullptr;
};
