// Copyright (c) Yevhenii Selivanov.

#include "FunctionPickerData/SWCFunctionPicker.h"

// Empty settings function
const FSWCFunctionPicker FSWCFunctionPicker::Empty = FSWCFunctionPicker();

// Custom constructor to set all members values
FSWCFunctionPicker::FSWCFunctionPicker(UClass* InFunctionClass, FName InFunctionName)
	: FunctionClass(InFunctionClass)
	, FunctionName(InFunctionName) {}

// Returns the function pointer based on set data to this structure
UFunction* FSWCFunctionPicker::GetFunction() const
{
	if (CachedFunctionInternal.IsValid())
	{
		return CachedFunctionInternal.Get();
	}

	if (FunctionClass
	    && !FunctionName.IsNone())
	{
		UFunction* FoundFunction = FunctionClass->FindFunctionByName(FunctionName, EIncludeSuperFlag::ExcludeSuper);
		CachedFunctionInternal = FoundFunction;
		return FoundFunction;
	}

	return nullptr;
}

// Compares for equality
bool FSWCFunctionPicker::operator==(const FSWCFunctionPicker& Other) const
{
	return Other.FunctionClass->IsChildOf(this->FunctionClass)
	       && Other.FunctionName == this->FunctionName;
}

// Creates a hash value
uint32 GetTypeHash(const FSWCFunctionPicker& Other)
{
	const uint32 FunctionClassHash = GetTypeHash(Other.FunctionClass);
	const uint32 FunctionNameHash = GetTypeHash(Other.FunctionName);
	return HashCombine(FunctionClassHash, FunctionNameHash);
}
