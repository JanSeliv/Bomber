// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "IPropertyTypeCustomization.h"

typedef class FMyPropertyTypeCustomization Super;


/**
 * Contains data that describes property.
 */
struct FPropertyData
{
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

	/**
	 * Set new template value to property handle.
	 * @tparam T Template param, is used to as to set simple types as well as set whole FProperty*
	 * @param NewValue
	 */
	template <typename T>
	void SetPropertyValueToHandle(const T& NewValue);
};

/**
 * Overrides some property to make better experience avoiding any errors in properties by manual typing etc.
 * The FName Property is customised as button to select the value in a list.
 */
class FMyPropertyTypeCustomization : public IPropertyTypeCustomization
{
public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/**
	 * Called when the header of the property (the row in the details panel where the property is shown)
	 * If nothing is added to the row, the header is not displayed
	 * @param PropertyHandle Handle to the property being customized
	 * @param HeaderRow	A row that widgets can be added to
	 * @param CustomizationUtils Utilities for customization
	 */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	/**
	* Called when the children of the property should be customized or extra rows added.
	* @param PropertyHandle Handle to the property being customized
	* @param ChildBuilder A builder for adding children
	* @param CustomizationUtils Utilities for customization
	*/
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	/** Get cached value contained in the property to be customized. */
	FORCEINLINE FText GetCustomPropertyValue() const { return FText::FromString(CustomProperty.PropertyValue.ToString()); }

	/** Set the FName value into the property.
	 * @see FMyPropertyTypeCustomization::MyPropertyHandleInternal. */
	void SetCustomPropertyValue(FName Value);

	/** Set true to activate property, false to grey out it (read-only). */
	void SetCustomPropertyEnabled(bool bEnabled);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Property data to be customized. It's property name has to be set in children's constructors.  */
	FPropertyData CustomProperty;

	/** The outer uobject of a property to be customized. */
	TWeakObjectPtr<class UObject> MyPropertyOuterInternal = nullptr;

	/** The text widget that displays the chosen property value. */
	TWeakPtr<class STextBlock> RowTextWidgetInternal = nullptr;

	/** Strings list of displayed values to be selected. */
	TArray<TSharedPtr<FString>> SearchableComboBoxValuesInternal;

	/** Contains the widget row that displays values to be selected.
	 * @see FMyPropertyTypeCustomization::SearchableComboBoxValuesInternal */
	TWeakPtr<class SSearchableComboBox> SearchableComboBoxInternal = nullptr;

	/** Contains data about all not custom child properties. */
	TArray<FPropertyData> DefaultPropertiesData;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/**
	 * Is called for each property on building its row.
	 * @param ChildBuilder A builder for adding children.
	 * @param PropertyData Data of a property to be customized.
	 */
	virtual void OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData);

	/**
	 * Is called on adding the custom property.
	 * @param PropertyDisplayText The formatted (with spacers) title name of a row to be shown.
	 * @param ChildBuilder A builder for adding children.
	 * @see FMyPropertyTypeCustomization::CustomPropertyNameInternal
	 */
	virtual void AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder);

	/** Set new values for the list of selectable members.
	 * @see FMyPropertyTypeCustomization::SearchableComboBoxValuesInternal */
	virtual void RefreshCustomProperty();

	/** Is called to deactivate custom property. */
	virtual void InvalidateCustomProperty();

	/** Returns true if changing custom property currently is not forbidden. */
	virtual bool IsAllowedEnableCustomProperty() const { return true; }

	/**
	 * Callback for when the function selection has changed from the dropdown. Will call setter of custom property.
	 * @param SelectedStringPtr The chosen string.
	 * @param SelectInfo Additional information about a selection event.
	 * @see FMyPropertyTypeCustomization::SetCustomPropertyValue(FName).
	 */
	void OnCustomPropertyChosen(TSharedPtr<FString> SelectedStringPtr, ESelectInfo::Type SelectInfo);
};
