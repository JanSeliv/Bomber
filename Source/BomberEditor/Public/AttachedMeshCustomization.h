// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "IPropertyTypeCustomization.h"

/**
 * Reuse the socket chooser widget to allow choose the FAttachedMesh::Bone within UPlayerRow::Mesh.
 */
class FAttachedMeshCustomization final : public IPropertyTypeCustomization
{
public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Makes a new instance of this detail layout class for a specific detail view requesting it. */
	static TSharedRef<class IPropertyTypeCustomization> MakeInstance();

	/**
	* Called when the header of the property (the row in the details panel where the property is shown)
	* If nothing is added to the row, the header is not displayed
	*
	* @param PropertyHandle			Handle to the property being customized
	* @param HeaderRow					A row that widgets can be added to
	* @param CustomizationUtils	Utilities for customization
	*/
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	/**
	* Called when the children of the property should be customized or extra rows added
	*
	* @param PropertyHandle			Handle to the property being customized
	* @param ChildBuilder				A builder for adding children
	* @param CustomizationUtils	Utilities for customization
	*/
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	/* ---------------------------------------------------
	*		Private properties
	* --------------------------------------------------- */

	/** FAttachedMesh::FName property. */
	TSharedPtr<IPropertyHandle> SocketPropertyHandleInternal = nullptr;

	/** UPlayerRow outer object. */
	TWeakObjectPtr<UObject> PlayerRowOuterInternal = nullptr;

	/** A Skeletal mesh component that contains the parent character mesh. Is transient component. User as a parameter to push the SSocketChooserPopup. */
	TWeakObjectPtr<USkeletalMeshComponent> ParentMeshCompInternal = nullptr;

	/* ---------------------------------------------------
	*		Private functions
	* --------------------------------------------------- */

	/** Customize a Socket property, will add the chosen text row, the Select and Clear buttons. */
	void AddSocketWidgetRow(IDetailChildrenBuilder& ChildBuilder);

	/** Push the SSocketChooserPopup menu to allow user choose socket. */
	void OnBrowseSocket();

	/** Set chosen socket to None. */
	void OnClearSocket();

	/** Executed every tick. */
	FText GetSocketFromProperty() const;

	/** Executed on socket selection. */
	void SetSocketToProperty(FName BoneName);
};
