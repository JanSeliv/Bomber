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
	CustomPropertyInternal.PropertyName = GET_MEMBER_NAME_CHECKED(FSettingsPicker, SettingsType);
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
			const FString SettingsDataBasePropertyName = SettingsPickerIt->GetName();
			SearchableComboBoxValuesInternal.Add(MakeShareable(new FString(SettingsDataBasePropertyName)));
		}
	}

	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
}

// Is called for each property on building its row
void FSettingsPickerCustomization::OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData)
{
	if (PropertyData.PropertyName != CustomPropertyInternal.PropertyName)
	{
		// Add lambda for visibility attribute to show\hide property when is not chosen
		PropertyData.Visibility = MakeAttributeLambda([InDefaultProperty = PropertyData, InCustomProperty = CustomPropertyInternal]() -> EVisibility
		{
			const FName OtherPropertyName = InDefaultProperty.PropertyName;
			const FName CustomPropertyValueName = InCustomProperty.GetPropertyValueFromHandle();
			return OtherPropertyName == CustomPropertyValueName ? EVisibility::Visible : EVisibility::Collapsed;
		});
	}
	Super::OnCustomizeChildren(ChildBuilder, PropertyData);
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
	// Super is not called to avoid refreshing searchable combo box, it always has the same values

	if (CachedSettingsTypeInternal.IsNone())
	{
		// Is called first time on init
		CachedSettingsTypeInternal = CustomPropertyInternal.PropertyValue;
		return;
	}

	// Compare with chosen option
	if (CachedSettingsTypeInternal != CustomPropertyInternal.PropertyValue)
	{
		OnNewSettingsTypeChosen();
	}
}

// Is called when is chosen new member from custom property
void FSettingsPickerCustomization::OnNewSettingsTypeChosen()
{
	FPropertyData PrevPropertyData = FPropertyData::Empty;
	FPropertyData NewPropertyData = FPropertyData::Empty;
	for (const FPropertyData& DefaultPropertyIt : DefaultPropertiesInternal)
	{
		if (DefaultPropertyIt.PropertyName == CustomPropertyInternal.PropertyValue)
		{
			NewPropertyData = DefaultPropertyIt;
		}
		else if (DefaultPropertyIt.PropertyName == CachedSettingsTypeInternal)
		{
			PrevPropertyData = DefaultPropertyIt;
		}
	}

	CachedSettingsTypeInternal = CustomPropertyInternal.PropertyValue;

	void* PrevValueData = nullptr;
	const FProperty* PrevProperty = nullptr;
	if (const IPropertyHandle* PrevPropertyHandle = PrevPropertyData.PropertyHandle.Get())
	{
		PrevPropertyHandle->GetValueData(PrevValueData);
		PrevProperty = PrevPropertyData.GetProperty();
	}

	void* NewValueData = nullptr;
	const FProperty* NewProperty = nullptr;
	if (const IPropertyHandle* NewPropertyHandle = NewPropertyData.PropertyHandle.Get())
	{
		NewPropertyHandle->GetValueData(NewValueData);
		NewProperty = NewPropertyData.GetProperty();
	}

	if (PrevValueData && PrevProperty)
	{
		if (NewValueData
		    && NewProperty)
		{
			NewProperty->ClearValue(NewValueData);
			NewProperty->CopyCompleteValue(NewValueData, PrevValueData);
		}

		PrevProperty->ClearValue(PrevValueData);
	}
}
