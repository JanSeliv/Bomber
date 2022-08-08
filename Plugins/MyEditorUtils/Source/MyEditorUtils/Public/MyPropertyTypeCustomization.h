// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "IPropertyTypeCustomization.h"
//---
#include "PropertyData.h"

typedef class FMyPropertyTypeCustomization Super;

/**
 * Overrides some property to make better experience avoiding any errors in properties by manual typing etc.
 * The FName Property is customised as button to select the value in a list.
 */
class MYEDITORUTILS_API FMyPropertyTypeCustomization : public IPropertyTypeCustomization
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
	FORCEINLINE FText GetCustomPropertyValue() const { return FText::FromString(CustomPropertyInternal.PropertyValue.ToString()); }

	/** Set the FName value into the property.
	 * @see FMyPropertyTypeCustomization::MyPropertyHandleInternal. */
	void SetCustomPropertyValue(FName Value);

	/** Set true to activate property, false to grey out it (read-only). */
	void SetCustomPropertyEnabled(bool bEnabled);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Contains data of hierarchically upper property which is chosen in editor module to customize its child property. */
	FPropertyData ParentPropertyInternal = FPropertyData::Empty;

	/** Property data to be customized. It's property name has to be set in children's constructors.  */
	FPropertyData CustomPropertyInternal = FPropertyData::Empty;

	/** Contains data about all not custom child properties. */
	TArray<FPropertyData> DefaultPropertiesInternal;

	/** The outer uobject of a property to be customized. */
	TWeakObjectPtr<class UObject> MyPropertyOuterInternal = nullptr;

	/** The text widget that displays the chosen property value. */
	TWeakPtr<class STextBlock> RowTextWidgetInternal = nullptr;

	/** Strings list of displayed values to be selected. */
	TArray<TSharedPtr<FString>> SearchableComboBoxValuesInternal;

	/** Contains the widget row that displays values to be selected.
	 * @see FMyPropertyTypeCustomization::SearchableComboBoxValuesInternal */
	TWeakPtr<class SSearchableComboBox> SearchableComboBoxInternal = nullptr;

	/** Shared none string. Is selectable value in the searchable box. */
	TWeakPtr<FString> NoneStringInternal = nullptr;

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

	/** Add an empty row once, so the users can clear the selection if they want. */
	void InitSearchableComboBox();

	/** Reset and remove all shared strings in array except 'None' string. */
	void ResetSearchableComboBox();
};
