// Copyright (c) Yevhenii Selivanov.

#pragma once

/**
 * Contains data that describes property.
 */
struct FPropertyData
{
	/** Empty property data. */
	static const FPropertyData Empty;

	/** The 'None' string. */
	static const FString& NoneString;

	/** Default empty constructor. */
	FPropertyData() = default;

	/** Custom constructor, is not required, but fully init property data. */
	explicit FPropertyData(TSharedRef<IPropertyHandle> InPropertyHandle);

	/** The name of a property. */
	FName PropertyName = NAME_None;

	/** The last cached value of a property. */
	FName PropertyValue = NAME_None;

	/** The handle of a property. */
	TSharedPtr<IPropertyHandle> PropertyHandle = nullptr;

	/** Determines if property is active (not greyed out). */
	TAttribute<bool> bIsEnabled = true;

	/** Determines if property is visible. */
	TAttribute<EVisibility> Visibility = EVisibility::Visible;

	/** Returns true is property is not empty. */
	FORCEINLINE bool IsValid() const { return !PropertyName.IsNone() && PropertyHandle != nullptr; }

	/** Get property from handle.*/
	FProperty* GetProperty() const;

	/** Get property name by handle.
	 * It returns current name of the property.
	 * Is cheaper to use cached one.
	 * @see FPropertyData::PropertyName. */
	FName GetPropertyNameFromHandle() const;

	/** Get property value by handle.
	* It returns current value contained in property.
	* Is cheaper to use cached one.
	* @see FPropertyData::PropertyValue. */
	FName GetPropertyValueFromHandle() const;

	/** Get property ptr to the value by handle. */
	void* GetPropertyValuePtrFromHandle() const;

	/**
	 * Set new template value to property handle.
	 * @tparam T Template param, is used to as to set simple types as well as set whole FProperty*
	 * @param NewValue Value to set.
	 */
	void SetPropertyValueToHandle(FName NewValue);

	/** Returns the meta value by specified key.
	 * @param Key The key of a meta to find. */
	FName GetMetaDataValue(FName Key) const;

	/** Returns true if specified key exist.
	* @param Key The key of a meta to check. */
	bool IsMetaKeyExists(FName Key) const;

	/** Set the meta value by specified key.
	* @param Key The key of a meta.
	* @param NewValue The value of a meta to set.
	* @param bNotifyPostChange Set true to notify property about change. */
	void SetMetaDataValue(FName Key, FName NewValue, bool bNotifyPostChange = false);
};
