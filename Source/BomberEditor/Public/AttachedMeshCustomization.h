// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "MyPropertyTypeCustomization.h"

/**
 * Reuse the socket chooser widget to allow choose the FAttachedMesh::Bone within UPlayerRow::Mesh.
 */
class FAttachedMeshCustomization final : public FMyPropertyTypeCustomization
{
public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** The name of class to be customized. */
	static const FName PropertyClassName;

	/** Default constructor. */
	FAttachedMeshCustomization();

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

	/** A Skeletal mesh component that contains the parent character mesh. Is transient component. Used as a parameter to push the SSocketChooserPopup. */
	TWeakObjectPtr<USkeletalMeshComponent> ParentMeshCompInternal = nullptr;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Is called for each property on building its row. */
	virtual void OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData) override;

	/** Is called on adding the custom property.
	 * Customize a Socket property, will add the chosen text row, the Select and Clear buttons.
	 * @see FMyPropertyTypeCustomization::CustomPropertyNameInternal */
	virtual void AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder) override;

	/** Push the SSocketChooserPopup menu to allow user choose socket. */
	void OnBrowseSocket();
};
