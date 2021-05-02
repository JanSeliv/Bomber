// Copyright 2021 Yevhenii Selivanov.

#include "SettingsFunctionCustomization.h"
//---
#include "Structures/SettingsRow.h"

typedef FSettingsFunctionCustomization ThisClass;

// The name of class to be customized
const FName ThisClass::PropertyClassName = FSettingsFunction::StaticStruct()->GetFName();

// Static const array, that contains names of metas used by this property
const ThisClass::FNamesArray ThisClass::TemplateMetaKeys =
{
	TEXT("SettingsFunctionContextTemplate"),
	TEXT("SettingsFunctionSetterTemplate"),
	TEXT("SettingsFunctionGetterTemplate")
};

// Default constructor
FSettingsFunctionCustomization::FSettingsFunctionCustomization()
{
	CustomPropertyInternal.PropertyName = GET_MEMBER_NAME_CHECKED(FSettingsFunction, FunctionName);
}

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FSettingsFunctionCustomization::MakeInstance()
{
	return MakeShareable(new ThisClass());
}

// Called when the header of the property (the row in the details panel where the property is shown)
void FSettingsFunctionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeHeader(PropertyHandle, HeaderRow, CustomizationUtils);
}

// Called when the children of the property should be customized or extra rows added.
void FSettingsFunctionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);

	InitTemplateMetaKey();

	RefreshCustomProperty();
}

// Is called for each property on building its row
void FSettingsFunctionCustomization::OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData)
{
	static const FName FunctionClassPropertyName = GET_MEMBER_NAME_CHECKED(FSettingsFunction, FunctionClass);
	if (PropertyData.PropertyName == FunctionClassPropertyName)
	{
		FunctionClassPropertyInternal = PropertyData;
	}

	Super::OnCustomizeChildren(ChildBuilder, PropertyData);
}

// Is called on adding the custom property
void FSettingsFunctionCustomization::AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder)
{
	// Super will add the searchable combo box
	Super::AddCustomPropertyRow(PropertyDisplayText, ChildBuilder);
}

// Set new values for the list of selectable members
void FSettingsFunctionCustomization::RefreshCustomProperty()
{
	// Invalidate if class is not chosen
	const FName ChosenFunctionClassName = FunctionClassPropertyInternal.GetPropertyValueFromHandle();
	if (ChosenFunctionClassName.IsNone())
	{
		InvalidateCustomProperty();
		return;
	}

	// Skip if nothing changed
	const bool bChosenNewClass = FunctionClassPropertyInternal.PropertyValue != ChosenFunctionClassName;
	FunctionClassPropertyInternal.PropertyValue = ChosenFunctionClassName;
	const bool bIsNewSettingsType = UpdateTemplateFunction();
	if (!bIsNewSettingsType && !bChosenNewClass    // Settings Type and Class depend on each other
	    && SearchableComboBoxValuesInternal.Num()) // List is not empty
	{
		return;
	}

	// Invalidate if invalid class
	const UClass* ChosenFunctionClass = GetChosenFunctionClass();
	if (!ChosenFunctionClass)
	{
		InvalidateCustomProperty();
		return;
	}

	SetCustomPropertyEnabled(true);

	ResetSearchableComboBox();

	TArray<FName> FoundList;
	bool bValidCustomProperty = false;
	for (TFieldIterator<UFunction> It(ChosenFunctionClass, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		const UFunction* FunctionIt = *It;
		if (FunctionIt
		    && FunctionIt != TemplateFunctionInternal
		    && (!bIsStaticFunctionInternal || FunctionIt->FunctionFlags & FUNC_Static) // only static functions if specified
		    && IsSignatureCompatible(FunctionIt))
		{
			FName FunctionNameIt = FunctionIt->GetFName();
			if (FunctionNameIt == CustomPropertyInternal.PropertyValue)
			{
				bValidCustomProperty = true;
			}

			FoundList.Emplace(MoveTemp(FunctionNameIt));
		}
	}

	// Reset function if does not contain in specified class
	if (!bValidCustomProperty)
	{
		SetCustomPropertyValue(NAME_None);
	}

	for (const FName& ItemData : FoundList)
	{
		// Add this to the searchable text box as an FString so users can type and find it
		SearchableComboBoxValuesInternal.Emplace(MakeShareable(new FString(ItemData.ToString())));
	}

	// Will refresh searchable combo box
	Super::RefreshCustomProperty();
}

// Is called to deactivate custom property
void FSettingsFunctionCustomization::InvalidateCustomProperty()
{
	Super::InvalidateCustomProperty();

	FunctionClassPropertyInternal.PropertyValue = NAME_None;
}

// Returns true if changing custom property currently is not forbidden
bool FSettingsFunctionCustomization::IsAllowedEnableCustomProperty() const
{
	return !FunctionClassPropertyInternal.PropertyValue.IsNone();
}

// Returns the currently chosen class of functions to display
const UClass* FSettingsFunctionCustomization::GetChosenFunctionClass() const
{
	const IPropertyHandle* ChildHandleIt = FunctionClassPropertyInternal.PropertyHandle.Get();
	const FProperty* ChildProperty = ChildHandleIt ? ChildHandleIt->GetProperty() : nullptr;
	const auto ObjectProperty = ChildProperty ? CastField<FObjectProperty>(ChildProperty) : nullptr;
	const uint8* Data = ObjectProperty ? ChildHandleIt->GetValueBaseAddress(nullptr) : nullptr;
	return Data ? Cast<UClass>(ObjectProperty->GetObjectPropertyValue(Data)) : nullptr;
}

// Check if all signatures of specified function are compatible with current template function
bool FSettingsFunctionCustomization::IsSignatureCompatible(const UFunction* Function) const
{
	if (!Function)
	{
		return false;
	}

	const UFunction* TemplateFunction = TemplateFunctionInternal.Get();
	if (!TemplateFunction)
	{
		// Template was not specified, so is compatible
		return true;
	}

	auto ArePropertiesTheSame = [](const FProperty* A, const FProperty* B)
	{
		if (A == B)
		{
			return true;
		}

		if (!A || !B) //one of properties is null
		{
			return false;
		}

		if (A->GetSize() != B->GetSize())
		{
			return false;
		}

		if (A->GetOffset_ForGC() != B->GetOffset_ForGC())
		{
			return false;
		}

		if (!A->SameType(B)) // A->GetClass() == B->GetClass()
		{
			// That part is implemented: if is return param with the same flags
			// Will return true for any derived UObject
			if (A->PropertyFlags & B->PropertyFlags & CPF_ReturnParm
			    && A->IsA(B->GetClass()))
			{
				return true;
			}
			return false;
		}

		return true;
	};

	const uint64 IgnoreFlags = CPF_OutParm | UFunction::GetDefaultIgnoredSignatureCompatibilityFlags();

	// Run thru the parameter property chains to compare each property
	TFieldIterator<FProperty> IteratorA(Function);
	TFieldIterator<FProperty> IteratorB(TemplateFunction);

	while (IteratorA && (IteratorA->PropertyFlags & CPF_Parm))
	{
		if (IteratorB && (IteratorB->PropertyFlags & CPF_Parm))
		{
			// Compare the two properties to make sure their types are identical
			// Note: currently this requires both to be strictly identical and wouldn't allow functions that differ only by how derived a class is,
			// which might be desirable when binding delegates, assuming there is directionality in the SignatureIsCompatibleWith call
			FProperty* PropA = *IteratorA;
			FProperty* PropB = *IteratorB;

			// Check the flags as well
			const uint64 PropertyMash = PropA->PropertyFlags ^ PropB->PropertyFlags;
			if (!ArePropertiesTheSame(PropA, PropB) || ((PropertyMash & ~IgnoreFlags) != 0))
			{
				// Type mismatch between an argument of A and B
				return false;
			}
		}
		else
		{
			// B ran out of arguments before A did
			return false;
		}
		++IteratorA;
		++IteratorB;
	}

	// They matched all the way thru A's properties, but it could still be a mismatch if B has remaining parameters
	return true;
}

// Set Template Function once
bool FSettingsFunctionCustomization::UpdateTemplateFunction()
{
	// Skip if meta was not changed
	const FName TemplateFunctionName = ParentPropertyInternal.GetMetaDataValue(TemplateMetaKeyInternal);
	if (TemplateFunctionName == TemplateMetaValueInternal)
	{
		return false;
	}
	TemplateMetaValueInternal = TemplateFunctionName;

	// Clear if meta is empty (none settings type is chosen)
	if (TemplateFunctionName.IsNone())
	{
		TemplateFunctionInternal.Reset();
		TemplateMetaValueInternal = NAME_None;
		return true;
	}

	static const FName ContextPropertyName = GET_MEMBER_NAME_CHECKED(FSettingsPrimary, StaticContext);
	if (ParentPropertyInternal.PropertyName == ContextPropertyName)
	{
		bIsStaticFunctionInternal = true;
	}

	// Set TemplateFunctionInternal to filter by its signature. All templates are stored in settings class
	const UObject* GameUserSettings = GEngine ? (UObject*)GEngine->GetGameUserSettings() : nullptr;
	const UClass* ScopeClass = GameUserSettings ? GameUserSettings->GetClass() : nullptr;
	TemplateFunctionInternal = ScopeClass ? ScopeClass->FindFunctionByName(TemplateFunctionName) : nullptr;
	ensureMsgf(TemplateFunctionInternal.IsValid(), TEXT("ASSERT: Specified '%s' function was not found in game settings"), *TemplateFunctionName.ToString());
	return true;
}

// Will set once the meta key of this property
void FSettingsFunctionCustomization::InitTemplateMetaKey()
{
	if (!TemplateMetaKeyInternal.IsNone() // set only once since the key never changes
	    || !ParentPropertyInternal.IsValid())
	{
		return;
	}

	// Will set once the meta key of this property.
	for (FName MetaKeyIt : TemplateMetaKeys)
	{
		if (ParentPropertyInternal.IsMetaKeyExists(MetaKeyIt))
		{
			TemplateMetaKeyInternal = MoveTemp(MetaKeyIt);
			break;
		}
	}
}
