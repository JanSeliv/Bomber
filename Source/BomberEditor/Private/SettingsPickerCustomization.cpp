// Copyright 2021 Yevhenii Selivanov

#include "SettingsPickerCustomization.h"
//---
#include "FunctionPickerCustomization.h"

typedef FSettingsPickerCustomization ThisClass;

// The name of class to be customized
const FName ThisClass::PropertyClassName = TEXT("SettingsPicker");

// The name of the settings data base struct
const FName ThisClass::SettingsDataBaseName = TEXT("SettingsDataBase");

// The name of the Settings Primary struct
const FName ThisClass::SettingsPrimaryName = TEXT("SettingsPrimary");

// The name of the Function Picker struct
const FName ThisClass::FunctionPickerName = TEXT("FunctionPicker");

// Default constructor
FSettingsPickerCustomization::FSettingsPickerCustomization()
{
	CustomPropertyInternal.PropertyName = TEXT("SettingsType");

	SettingsFunctionProperties.Reserve(FFunctionPickerCustomization::TemplateMetaKeys.Num());
	for (FName MetaNameIt : FFunctionPickerCustomization::TemplateMetaKeys)
	{
		SettingsFunctionProperties.Emplace(MoveTemp(MetaNameIt), FPropertyData::Empty);
	}
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

	// Display added values in the picker list
	Super::RefreshCustomProperty();
	ClearPrevChosenProperty();
}

// Is called for each property on building its row
void FSettingsPickerCustomization::OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData)
{
	/* ╔FSettingsPicker
	 * ╠════╦FSettingsPrimary (1)
	 * ║	╚════FFunctionPicker (2)
	 * ╚═════FSettingsDataBase (3)
	 */

	if (!SettingsDataBaseStructInternal.IsValid())
	{
		SettingsDataBaseStructInternal = !SettingsDataBaseName.IsNone() ? FindObject<UScriptStruct>(ANY_PACKAGE, *SettingsDataBaseName.ToString(), true) : nullptr;
	}

	if (!SettingsPrimaryStructInternal.IsValid())
	{
		SettingsPrimaryStructInternal = !SettingsDataBaseName.IsNone() ? FindObject<UScriptStruct>(ANY_PACKAGE, *SettingsPrimaryName.ToString(), true) : nullptr;
	}

	if (!FunctionPickerStructInternal.IsValid())
	{
		FunctionPickerStructInternal = !SettingsDataBaseName.IsNone() ? FindObject<UScriptStruct>(ANY_PACKAGE, *FunctionPickerName.ToString(), true) : nullptr;
	}

	const auto StructProperty = CastField<FStructProperty>(PropertyData.GetProperty());
	const UScriptStruct* StructClass = StructProperty ? StructProperty->Struct : nullptr;
	if (StructClass)
	{
		if (StructClass->IsChildOf(SettingsPrimaryStructInternal.Get())) //(1)
		{
			// Add lambda for visibility attribute to show\hide function properties when nothing is chosen
			PropertyData.Visibility = MakeAttributeLambda([InCustomProperty = CustomPropertyInternal]() -> EVisibility
			{
				const FName CustomPropertyValueName = InCustomProperty.GetPropertyValueFromHandle();
				return !CustomPropertyValueName.IsNone() ? EVisibility::Visible : EVisibility::Collapsed;
			});

			const TSharedRef<IPropertyHandle>& SettingsPrimaryHandle = PropertyData.PropertyHandle.ToSharedRef();
			uint32 NumChildren;
			SettingsPrimaryHandle->GetNumChildren(NumChildren);
			for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
			{
				FPropertyData SettingsPrimaryData(SettingsPrimaryHandle->GetChildHandle(ChildIndex).ToSharedRef());
				const FStructProperty* ChildProperty = CastField<FStructProperty>(SettingsPrimaryData.GetProperty());
				const UScriptStruct* ChildClass = ChildProperty ? ChildProperty->Struct : nullptr;
				if (ChildClass
				    && ChildClass->IsChildOf(FunctionPickerStructInternal.Get())) //(2)
				{
					// Find and set function properties into array
					for (TTuple<FName, FPropertyData>& SettingsFunctionPropertyIt : SettingsFunctionProperties)
					{
						const FName MetaKey = SettingsFunctionPropertyIt.Key;
						if (SettingsPrimaryData.IsMetaKeyExists(MetaKey))
						{
							SettingsFunctionPropertyIt.Value = MoveTemp(SettingsPrimaryData);
							break;
						}
					}
				}
			}
		}
		else if (StructClass->IsChildOf(SettingsDataBaseStructInternal.Get())) //(3)
		{
			// Add this to the searchable text box as an FString so users can type and find it
			const FString SettingsDataBasePropertyName = PropertyData.PropertyName.ToString();
			SearchableComboBoxValuesInternal.Emplace(MakeShareable(new FString(SettingsDataBasePropertyName)));

			// Add lambda for visibility attribute to show\hide property when is not chosen
			PropertyData.Visibility = MakeAttributeLambda([InDefaultProperty = PropertyData, InCustomProperty = CustomPropertyInternal]() -> EVisibility
			{
				const FName OtherPropertyName = InDefaultProperty.PropertyName;
				const FName CustomPropertyValueName = InCustomProperty.GetPropertyValueFromHandle();
				return OtherPropertyName == CustomPropertyValueName ? EVisibility::Visible : EVisibility::Collapsed;
			});
		}
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

	// Compare with chosen option
	if (ChosenSettingsDataInternal.PropertyName != CustomPropertyInternal.PropertyValue)
	{
		ClearPrevChosenProperty();
	}
}

// Will reset the whole property content of last chosen settings type
void FSettingsPickerCustomization::ClearPrevChosenProperty()
{
	FPropertyData NewPropertyData = FPropertyData::Empty;
	FPropertyData PrevPropertyData = FPropertyData::Empty;
	for (const FPropertyData& DefaultPropertyIt : DefaultPropertiesInternal)
	{
		if (!NewPropertyData.IsValid()
		    && CustomPropertyInternal.IsValid()
		    && CustomPropertyInternal.PropertyValue == DefaultPropertyIt.PropertyName)
		{
			NewPropertyData = DefaultPropertyIt;
			continue;
		}

		if (!PrevPropertyData.IsValid()
		    && ChosenSettingsDataInternal.IsValid()
		    && ChosenSettingsDataInternal.PropertyName == DefaultPropertyIt.PropertyName)
		{
			PrevPropertyData = DefaultPropertyIt;
		}
	}

	if (!PrevPropertyData.IsValid()
	    && !NewPropertyData.IsValid())
	{
		return;
	}

	ChosenSettingsDataInternal = NewPropertyData;

	// Clear prev value
	void* PrevValueData = PrevPropertyData.GetPropertyValuePtrFromHandle();
	const FProperty* PrevProperty = PrevPropertyData.GetProperty();
	if (PrevValueData
	    && PrevProperty)
	{
		PrevProperty->ClearValue(PrevValueData);
	}

	CopyMetas();
}

// Copy meta from chosen option USTRUCT to each UPROPERTY of Settings Functions
void FSettingsPickerCustomization::CopyMetas()
{
	const auto StructProperty = CastField<FStructProperty>(ChosenSettingsDataInternal.GetProperty());
	const UScriptStruct* SettingsDataChildStruct = StructProperty ? StructProperty->Struct : nullptr;

	for (TTuple<FName, FPropertyData>& PropertyIt : SettingsFunctionProperties)
	{
		const FName TemplateMetaKey = PropertyIt.Key;
		FPropertyData& PropertyDataRef = PropertyIt.Value;
		if (!PropertyIt.Value.IsValid())
		{
			continue;
		}

		if (!SettingsDataChildStruct)
		{
			PropertyDataRef.SetMetaDataValue(TemplateMetaKey, NAME_None, true);
			continue;
		}

		// Check meta key in parent struct if not found for child struct
		for (const UStruct* StructIt = SettingsDataChildStruct; StructIt; StructIt = StructIt->GetSuperStruct())
		{
			// Copy meta from chosen option USTRUCT to each UPROPERTY of SettingsFunctionProperties
			if (const FString* FoundMetaData = StructIt->FindMetaData(TemplateMetaKey))
			{
				PropertyDataRef.SetMetaDataValue(TemplateMetaKey, **FoundMetaData, true);
				break;
			}

			if (StructIt == SettingsDataBaseStructInternal)
			{
				// The meta key was not found
				PropertyDataRef.SetMetaDataValue(TemplateMetaKey, NAME_None, true);
				break;
			}
		}
	}
}
