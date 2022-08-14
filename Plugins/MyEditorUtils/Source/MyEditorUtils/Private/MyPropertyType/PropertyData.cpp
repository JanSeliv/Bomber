// Copyright (c) Yevhenii Selivanov.

#include "MyPropertyType/PropertyData.h"

// Empty property data
const FPropertyData FPropertyData::Empty = FPropertyData();

// Custom constructor, is not required, but fully init property data.
FPropertyData::FPropertyData(TSharedRef<IPropertyHandle> InPropertyHandle)
	: PropertyHandle(InPropertyHandle)
{
	PropertyName = GetPropertyNameFromHandle();
	PropertyValue = GetPropertyValueFromHandle();
}

// Get property from handle
FProperty* FPropertyData::GetProperty() const
{
	return PropertyHandle ? PropertyHandle->GetProperty() : nullptr;
}

// Get property name by handle
FName FPropertyData::GetPropertyNameFromHandle() const
{
	const FProperty* CurrentProperty = GetProperty();
	return CurrentProperty ? CurrentProperty->GetFName() : NAME_None;
}

// Get FName value by property handle
FName FPropertyData::GetPropertyValueFromHandle() const
{
	FName ValueName = NAME_None;
	if (PropertyHandle.IsValid())
	{
		FString ValueString;
		PropertyHandle->GetValueAsDisplayString(/*Out*/ValueString);
		if (ValueString.Len() < NAME_SIZE)
		{
			ValueName = *ValueString;
		}
	}
	return ValueName;
}

// Get property ptr to the value by handle
void* FPropertyData::GetPropertyValuePtrFromHandle() const
{
	void* FoundData = nullptr;
	if (PropertyHandle.IsValid())
	{
		PropertyHandle->GetValueData(/*Out*/FoundData);
	}
	return FoundData;
}

// Set FName value by property handle
void FPropertyData::SetPropertyValueToHandle(FName NewValue)
{
	if (PropertyHandle.IsValid())
	{
		PropertyHandle->SetValue(NewValue);
	}
}

// Returns the meta value by specified ke
FName FPropertyData::GetMetaDataValue(FName Key) const
{
	const FProperty* Property = !Key.IsNone() ? GetProperty() : nullptr;
	const FString* FoundKey = Property ? Property->FindMetaData(Key) : nullptr;
	return FoundKey ? **FoundKey : *NoneString;
}

// Returns true if specified key exist
bool FPropertyData::IsMetaKeyExists(FName Key) const
{
	const FProperty* Property = !Key.IsNone() ? GetProperty() : nullptr;
	return Property && Property->FindMetaData(Key);
}

// Set the meta value by specified key
void FPropertyData::SetMetaDataValue(FName Key, FName NewValue, bool bNotifyPostChange/* = false*/)
{
	FProperty* Property = !Key.IsNone() ? GetProperty() : nullptr;
	if (!Property)
	{
		return;
	}

	const FName PrevMetaValue = GetMetaDataValue(Key);
	if (PrevMetaValue == NewValue)
	{
		return;
	}

	Property->SetMetaData(Key, NewValue.ToString());

	if (bNotifyPostChange)
	{
		PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}
