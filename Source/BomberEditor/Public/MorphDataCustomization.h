// Copyright 2021 Yevhenii Selivanov.

#pragma once

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

	/** The name of class to be customized. */
	static const FName PropertyClassName;

	/** Default constructor. */
	FMorphDataCustomization();

	/** Makes a new instance of this detail layout class for a specific detail view requesting it. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

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

	/** A name of the last mesh. When is not changed, refreshing of a custom property will be skipped.
	* @see FMorphDataCustomization::RefreshCustomProperty() */
	FName CachedMeshNameInternal = NAME_None;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Is called for each property on building its row. */
	virtual void OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, const FPropertyData& PropertyData) override;

	/** Is called on adding the custom property.
	* @see FMyPropertyTypeCustomization::CustomPropertyNameInternal */
	virtual void AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder) override;

	/** Set new values for the list of selectable members.
	* @see FMyPropertyTypeCustomization::SearchableComboBoxValuesInternal */
	virtual void RefreshCustomProperty() override;
};
