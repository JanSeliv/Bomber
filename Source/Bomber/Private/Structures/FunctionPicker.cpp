#include "Structures/FunctionPicker.h"

// Empty settings function
const FFunctionPicker FFunctionPicker::Empty = FFunctionPicker();

// Custom constructor to set all members values
FFunctionPicker::FFunctionPicker(UClass* InFunctionClass, FName InFunctionName)
	: FunctionClass(InFunctionClass)
	, FunctionName(InFunctionName) {}

// Compares for equality
bool FFunctionPicker::operator==(const FFunctionPicker& Other) const
{
	return Other.FunctionClass->IsChildOf(this->FunctionClass)
	       && Other.FunctionName == this->FunctionName;
}

// Creates a hash value
uint32 GetTypeHash(const FFunctionPicker& Other)
{
	const uint32 FunctionClassHash = GetTypeHash(Other.FunctionClass);
	const uint32 FunctionNameHash = GetTypeHash(Other.FunctionName);
	return HashCombine(FunctionClassHash, FunctionNameHash);
}
