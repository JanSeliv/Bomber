// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "MyPropertyTypeCustomization.h"

/**
 * Is customized to show only selected in-game option.
 */
class FSettingsPickerCustomization final : public FMyPropertyTypeCustomization
{
public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** The name of class to be customized. */
	static const FName PropertyClassName;

	/** Pointer to the FSettingsDataBase struct. */
	static const UScriptStruct* const& SettingsDataBaseStruct;

	/** Default constructor. */
	FSettingsPickerCustomization();

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

	/** One of chosen FSettingsDataBase property. */
	FPropertyData ChosenSettingsDataInternal = FPropertyData::Empty;

	/** Contains property to by meta.
	 * MetaName: SettingsFunctionContextTemplate, SettingsFunctionSetterTemplate, SettingsFunctionGetterTemplate.
	 * SettingsFunctionProperty: FSettingsPicker::StaticContext, FSettingsPicker::Setter, FSettingsPicker::Getter, */
	TMap<FName/*TemplateMetaKey*/, FPropertyData/*SettingsFunctionProperty*/> SettingsFunctionProperties;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Is called for each property on building its row. */
	virtual void OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData) override;

	/** Is called on adding the custom property.
	* @see FMyPropertyTypeCustomization::CustomPropertyNameInternal */
	virtual void AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder) override;

	/** Is called when any child property is changed. */
	virtual void RefreshCustomProperty() override;

	/** Will reset the whole property content of last chosen settings type. */
	void ClearPrevChosenProperty();

	/** Copy meta from chosen option USTRUCT to each UPROPERTY of Settings Functions.
     * @see FMyPropertyTypeCustomization::CustomPropertyInternal
     * @see FSettingsPickerCustomization::SettingsFunctionProperties */
	void CopyMetas();
};
