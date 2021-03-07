// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "IPropertyTypeCustomization.h"
#include "MyPropertyTypeCustomization.h"

/**
 * Allow to choose the function for UGameUserSettings instead of manually typing a name.
 */
class FSettingsFunctionCustomization final : public FMyPropertyTypeCustomization
{
public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Default constructor. */
	FSettingsFunctionCustomization();

	/** Makes a new instance of this detail layout class for a specific detail view requesting it. */
	static TSharedRef<class IPropertyTypeCustomization> MakeInstance();

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

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** The handle of a property that contains a class of functions to display.
	 * @see FSettingsFunction::Class */
	TSharedPtr<IPropertyHandle> FunctionClassHandleInternal = nullptr;

	/** Contains the function to be compared with all other functions of a class to show in the list only compatible functions.
	 * @see FSettingsFunctionCustomization::RefreshCustomProperty() */
	TWeakObjectPtr<UFunction> TemplateSetterInternal = nullptr;

	/** A name of last chosen class property. When is not changed, refreshing of a custom property will be skipped.
	 * @see FSettingsFunctionCustomization::RefreshCustomProperty() */
	FName CachedFunctionClassInternal = NAME_None;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Is called for each property on building its row. */
	virtual void OnCustomizeChildren(TSharedRef<IPropertyHandle> ChildPropertyHandle, IDetailChildrenBuilder& ChildBuilder, FName PropertyName) override;

	/** Is called on adding the custom property.
	* @see FMyPropertyTypeCustomization::CustomPropertyNameInternal */
	virtual void AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder) override;

	/** Set new values for the list of selectable members.
	* @see FMyPropertyTypeCustomization::SearchableComboBoxValuesInternal */
	virtual void RefreshCustomProperty() override;

	/** Is called to deactivate custom property. */
	virtual void InvalidateCustomProperty() override;

	/** Returns true if changing custom property currently is not forbidden. */
	virtual bool IsAllowedEnableCustomProperty() const override;

	/** Is called on changing value of any child property. */
	virtual void OnAnyChildPropertyChanged() override;

	/** Returns the currently chosen class of functions to display.
	 * @see FSettingsFunctionCustomization::FunctionClassHandleInternal
	 * @see FSettingsFunction::Class */
	const UClass* GetChosenFunctionClass() const;
};
