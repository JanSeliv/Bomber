// Copyright 2021 Yevhenii Selivanov

#include "SettingsPickerCustomization.h"
//---
#include "Structures/SettingsRow.h"

typedef FSettingsPickerCustomization ThisClass;

// The name of class to be customized
const FName FSettingsPickerCustomization::PropertyClassName = FSettingsPicker::StaticStruct()->GetFName();

// Default constructor
FSettingsPickerCustomization::FSettingsPickerCustomization()
{
	CustomPropertyNameInternal = GET_MEMBER_NAME_CHECKED(FSettingsPicker, SettingsType);
}

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FSettingsPickerCustomization::MakeInstance()
{
	return MakeShareable(new ThisClass());
}

// Called when the header of the property (the row in the details panel where the property is shown)
void FSettingsPickerCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Do not use the header panel at all
}

// Called when the children of the property should be customized or extra rows added.
void FSettingsPickerCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
}

// Is called for each property on building its row
void FSettingsPickerCustomization::OnCustomizeChildren(TSharedRef<IPropertyHandle> ChildPropertyHandle, IDetailChildrenBuilder& ChildBuilder, FName PropertyName)
{
	static const FName PropertySelector = GET_MEMBER_NAME_CHECKED(FSettingsPicker, SettingsType);

	const FName SelectedProperty = *GetCustomPropertyDisplayText().ToString();
	if (PropertyName == SelectedProperty
	    || PropertyName == PropertySelector)
	{
		// Add only chosen property
		Super::OnCustomizeChildren(ChildPropertyHandle, ChildBuilder, PropertyName);
	}
}

// Is called on adding the custom property
void FSettingsPickerCustomization::AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder)
{
	// Super will add the searchable combo box
	Super::AddCustomPropertyRow(PropertyDisplayText, ChildBuilder);
}

// Set new values for the list of selectable members
void FSettingsPickerCustomization::RefreshCustomProperty()
{
	const FName SettingsType = *GetCustomPropertyDisplayText().ToString();
	if (SettingsType == CachedSettingsTypeInternal)
	{
		return;
	}
	CachedSettingsTypeInternal = SettingsType;

	SearchableComboBoxValuesInternal.Empty();

	// Add an empty row, so the user can clear the selection if they want
	static const FString NoneName{FName(NAME_None).ToString()};
	const TSharedPtr<FString> NoneNamePtr = MakeShareable(new FString(NoneName));
	SearchableComboBoxValuesInternal.Add(NoneNamePtr);

	static const UScriptStruct* const SettingsDataBaseStruct = FSettingsDataBase::StaticStruct();
	static const UScriptStruct* const SettingsPickerStruct = FSettingsPicker::StaticStruct();
	for (TFieldIterator<FStructProperty> SettingsPickerIt(SettingsPickerStruct); SettingsPickerIt; ++SettingsPickerIt)
	{
		const UScriptStruct* StructPropertyIt = SettingsPickerIt->Struct;
		if (StructPropertyIt
		    && StructPropertyIt->IsChildOf(SettingsDataBaseStruct))
		{
			// Add this to the searchable text box as an FString so users can type and find it
			const FString MorphTarget = SettingsPickerIt->GetName();
			SearchableComboBoxValuesInternal.Add(MakeShareable(new FString(MorphTarget)));
		}
	}

	// Will refresh searchable combo box
	Super::RefreshCustomProperty();
}
