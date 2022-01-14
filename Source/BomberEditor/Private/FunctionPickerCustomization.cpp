// Copyright 2021 Yevhenii Selivanov.

#include "FunctionPickerCustomization.h"

typedef FFunctionPickerCustomization ThisClass;

// The name of class to be customized
const FName ThisClass::PropertyClassName = TEXT("FunctionPicker");

// Static const array, that contains names of metas used by this property
const ThisClass::FNamesArray ThisClass::TemplateMetaKeys =
{
	TEXT("FunctionContextTemplate"),
	TEXT("FunctionSetterTemplate"),
	TEXT("FunctionGetterTemplate")
};

// Default constructor
FFunctionPickerCustomization::FFunctionPickerCustomization()
{
	CustomPropertyInternal.PropertyName = TEXT("FunctionName");
}

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FFunctionPickerCustomization::MakeInstance()
{
	return MakeShareable(new ThisClass());
}

// Called when the header of the property (the row in the details panel where the property is shown)
void FFunctionPickerCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeHeader(PropertyHandle, HeaderRow, CustomizationUtils);
}

// Called when the children of the property should be customized or extra rows added.
void FFunctionPickerCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);

	InitTemplateMetaKey();

	RefreshCustomProperty();
}

// Is called for each property on building its row
void FFunctionPickerCustomization::OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData)
{
	static const FName FunctionClassPropertyName = TEXT("FunctionClass");
	if (PropertyData.PropertyName == FunctionClassPropertyName)
	{
		FunctionClassPropertyInternal = PropertyData;
	}

	Super::OnCustomizeChildren(ChildBuilder, PropertyData);
}

// Is called on adding the custom property
void FFunctionPickerCustomization::AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder)
{
	// Super will add the searchable combo box
	Super::AddCustomPropertyRow(PropertyDisplayText, ChildBuilder);
}

// Set new values for the list of selectable members
void FFunctionPickerCustomization::RefreshCustomProperty()
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
	const bool bIsNewTemplateFunction = UpdateTemplateFunction();
	if (!bIsNewTemplateFunction && !bChosenNewClass // Settings Type and Class depend on each other
	    && SearchableComboBoxValuesInternal.Num())  // List is not empty
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
void FFunctionPickerCustomization::InvalidateCustomProperty()
{
	Super::InvalidateCustomProperty();

	FunctionClassPropertyInternal.PropertyValue = NAME_None;
}

// Returns true if changing custom property currently is not forbidden
bool FFunctionPickerCustomization::IsAllowedEnableCustomProperty() const
{
	return !FunctionClassPropertyInternal.PropertyValue.IsNone();
}

// Returns the currently chosen class of functions to display
const UClass* FFunctionPickerCustomization::GetChosenFunctionClass() const
{
	const IPropertyHandle* ChildHandleIt = FunctionClassPropertyInternal.PropertyHandle.Get();
	const FProperty* ChildProperty = ChildHandleIt ? ChildHandleIt->GetProperty() : nullptr;
	const FObjectProperty* const ObjectProperty = ChildProperty ? CastField<FObjectProperty>(ChildProperty) : nullptr;
	uint8* Base = reinterpret_cast<uint8*>(MyPropertyOuterInternal.Get());
	const uint8* Data = ObjectProperty ? ChildHandleIt->GetValueBaseAddress(Base) : nullptr;
	return Data ? Cast<UClass>(ObjectProperty->GetObjectPropertyValue(Data)) : nullptr;
}

// Check if all signatures of specified function are compatible with current template function
bool FFunctionPickerCustomization::IsSignatureCompatible(const UFunction* Function) const
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

	// Run through the parameter property chains to compare each property
	TFieldIterator<FProperty> IteratorA(TemplateFunction);
	TFieldIterator<FProperty> IteratorB(Function);

	while (IteratorA && (IteratorA->PropertyFlags & CPF_Parm))
	{
		if (IteratorB && (IteratorB->PropertyFlags & CPF_Parm))
		{
			// Compare the two properties to make sure their types are identical
			// Note: currently this requires both to be strictly identical and wouldn't allow functions that differ only by how derived a class is,
			// which might be desirable when binding delegates, assuming there is directionality in the SignatureIsCompatibleWith call
			const FProperty* PropA = *IteratorA;
			const FProperty* PropB = *IteratorB;

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

	// They matched all the way through A's properties, but it could still be a mismatch if B has remaining parameters
	return true;
}

// Set Template Function once
bool FFunctionPickerCustomization::UpdateTemplateFunction()
{
	// Skip if meta was not changed
	const FName TemplateMetaValue = ParentPropertyInternal.GetMetaDataValue(TemplateMetaKeyInternal);
	if (TemplateMetaValue == TemplateMetaValueInternal)
	{
		return true;
	}
	TemplateMetaValueInternal = TemplateMetaValue;

	// Clear if meta is empty (none settings type is chosen)
	if (TemplateMetaValue.IsNone())
	{
		TemplateFunctionInternal.Reset();
		TemplateMetaValueInternal = NAME_None;
		return true;
	}

	// Parse into class name and function name
	TArray<FString> ParsedStrArray;
	static const FString Delimiter(TEXT("::"));
	TemplateMetaValue.ToString().ParseIntoArray(ParsedStrArray, *Delimiter);

	// Find class
	static constexpr int32 ClassNameIndex = 0;
	const FString ClassName = ParsedStrArray.IsValidIndex(ClassNameIndex) ? ParsedStrArray[ClassNameIndex] : TEXT("");
	const UClass* ScopeClass = !ClassName.IsEmpty() ? FindObject<UClass>(ANY_PACKAGE, *ClassName, true) : nullptr;
	if (!ensureMsgf(ScopeClass, TEXT("ASSERT: 'ScopeClass' is not valid")))
	{
		return false;;
	}

	// Find function to filter by its signature
	static constexpr int32 FunctionNameIndex = 1;
	const FString FunctionName = ParsedStrArray.IsValidIndex(FunctionNameIndex) ? ParsedStrArray[FunctionNameIndex] : TEXT("");
	TemplateFunctionInternal = !FunctionName.IsEmpty() ? ScopeClass->FindFunctionByName(*FunctionName) : nullptr;
	if (!ensureMsgf(TemplateFunctionInternal.IsValid(), TEXT("ASSERT: Template function was not found")))
	{
		return false;
	}

	return true;
}

// Will set once the meta key of this property
void FFunctionPickerCustomization::InitTemplateMetaKey()
{
	if (!TemplateMetaKeyInternal.IsNone() // set only once since the key never changes
	    || !ParentPropertyInternal.IsValid())
	{
		return;
	}

	// Will set once the meta key of this property.
	for (const FName MetaKeyIt : TemplateMetaKeys)
	{
		if (ParentPropertyInternal.IsMetaKeyExists(MetaKeyIt))
		{
			TemplateMetaKeyInternal = MetaKeyIt;
			break;
		}
	}

	// Set if function is static
	static const FName FunctionContextTemplate = TEXT("FunctionContextTemplate");
	bIsStaticFunctionInternal = TemplateMetaKeyInternal == FunctionContextTemplate;
}
