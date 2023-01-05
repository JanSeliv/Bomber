// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "SWCFunctionPicker.generated.h"

/**
  * Allows designer to choose the function in the list.
  *
  * It's possible to set the template by specifying the function or delegate in the UPROPERTY's meta.
  * It will filter the list of all functions by its return type and type of all function arguments.
  *	If none meta is set, all functions will appear in the list without any filtering.
  *
  * Meta keys:
  * FunctionContextTemplate (only to show static functions or Blueprint Library functions)
  * FunctionGetterTemplate
  * FunctionSetterTemplate
  *
  * Meta value (without prefixes):
  * /Script/ModuleName.ClassName::FunctionName
  *
  * Example:
  * UPROPERTY(EditDefaultsOnly, meta = (FunctionSetterTemplate = "/Script/FunctionPicker.FunctionPickerTemplate::OnSetMembers__DelegateSignature"))
  * FFunctionPicker SetMembers = FFunctionPicker::Empty;
  */
USTRUCT(BlueprintType)
struct FSWCFunctionPicker
{
	GENERATED_BODY()

	/** Empty function data. */
	static const FSWCFunctionPicker Empty;

	/** Default constructor. */
	FSWCFunctionPicker() = default;

	/** Custom constructor to set all members values. */
	FSWCFunctionPicker(UClass* InFunctionClass, FName InFunctionName);

	/** The class where function can be found. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Class"))
	TObjectPtr<UClass> FunctionClass = nullptr;

	/** The function name to choose for specified class.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Function"))
	FName FunctionName = NAME_None;

	/** Returns true if is valid. */
	FORCEINLINE bool IsValid() const { return !(*this == Empty); }

	/** Returns the function pointer based on set data to this structure. */
	UFunction* GetFunction() const;

	/** Compares for equality.
	  * @param Other The other object being compared. */
	bool operator==(const FSWCFunctionPicker& Other) const;

	/** Creates a hash value.
	  * @param Other the other object to create a hash value for. */
	friend uint32 GetTypeHash(const FSWCFunctionPicker& Other);

protected:
	/** Contains cached function ptr for performance reasons. */
	mutable TWeakObjectPtr<UFunction> CachedFunctionInternal = nullptr;
};
