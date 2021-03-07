// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "IPropertyTypeCustomization.h"
#include "MyPropertyTypeCustomization.h"

/**
 * Allow to choose the morph for FMorphData::Morph within UAnimNotify instead of manually typing a name.
 */
class FMorphDataCustomization final : public FMyPropertyTypeCustomization
{
public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Default constructor. */
	FMorphDataCustomization();

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
};
