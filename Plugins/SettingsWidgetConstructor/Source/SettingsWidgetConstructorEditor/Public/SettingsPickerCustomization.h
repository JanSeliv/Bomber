// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "MyPropertyType/MyPropertyTypeCustomization.h"

/**
 * Is customized to show only selected in-game option.
 */
class SETTINGSWIDGETCONSTRUCTOREDITOR_API FSettingsPickerCustomization : public FMyPropertyTypeCustomization
{
public:
	/** The name of class to be customized: SettingsPicker */
	static const FName PropertyClassName;

	/** The name of the settings data base struct: /Script/SettingsWidgetConstructor.SettingsDataBase */
	static const FName SettingsDataBasePathName;

	/** The name of the Settings Primary struct: /Script/SettingsWidgetConstructor.SettingsPrimary */
	static const FName SettingsPrimaryPathName;

	/** The name of the Function Picker struct: /Script/FunctionPicker.SWCFunctionPicker */
	static const FName FunctionPickerPathName;

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

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

	/** Creates customization for the Settings Picker. */
	static void RegisterSettingsPickerCustomization();

	/** Removes customization for the Settings Picker. */
	static void UnregisterSettingsPickerCustomization();

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

	/** Pointer to the FSettingsDataBase struct. */
	TWeakObjectPtr<UScriptStruct> SettingsDataBaseStructInternal = nullptr;

	/** Pointer to the SettingsPrimary struct. */
	TWeakObjectPtr<UScriptStruct> SettingsPrimaryStructInternal = nullptr;

	/** Pointer to the FunctionPicker struct. */
	TWeakObjectPtr<UScriptStruct> FunctionPickerStructInternal = nullptr;

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
