// Copyright 2021 Yevhenii Selivanov.

#include "SettingsFunctionCustomization.h"
//---
#include "Structures/SettingsRow.h"

typedef FSettingsFunctionCustomization ThisClass;

// The name of class to be customized
const FName FSettingsFunctionCustomization::PropertyClassName = FSettingsFunction::StaticStruct()->GetFName();

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
	InitTemplateFunction(PropertyHandle);

	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
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
	const UFunction* TemplateFunction = TemplateFunctionInternal.Get();
	if (!TemplateFunction)
	{
		InvalidateCustomProperty();
		return;
	}

	const UClass* ChosenFunctionClass = GetChosenFunctionClass();
	if (!ChosenFunctionClass)
	{
		InvalidateCustomProperty();
		return;
	}

	// Compare with cached class, if equal and combo box values exist, then skip refreshing
	const FName ChosenFunctionClassName = FunctionClassPropertyInternal.GetPropertyValueFromHandle();
	if (ChosenFunctionClassName == FunctionClassPropertyInternal.PropertyValue
	    && SearchableComboBoxValuesInternal.Num())
	{
		return;
	}
	FunctionClassPropertyInternal.PropertyValue = ChosenFunctionClassName;

	SetCustomPropertyEnabled(true);

	SearchableComboBoxValuesInternal.Empty();

	// Add an empty row, so the user can clear the selection if they want
	static const FString NoneName{FName(NAME_None).ToString()};
	const TSharedPtr<FString> NoneNamePtr = MakeShareable(new FString(NoneName));
	SearchableComboBoxValuesInternal.Add(NoneNamePtr);

	TArray<FName> FoundList;
	TArray<FName> AllChildFunctions;
	bool bValidCustomProperty = false;
	ChosenFunctionClass->GenerateFunctionList(AllChildFunctions);
	for (TFieldIterator<UFunction> It(ChosenFunctionClass); It; ++It)
	{
		const UFunction* FunctionIt = *It;
		if (FunctionIt
		    && FunctionIt != TemplateFunction
		    && (!bIsStaticFunctionInternal || FunctionIt->FunctionFlags & FUNC_Static) // only static functions if specified
		    && IsSignatureCompatible(FunctionIt))
		{
			FName FunctionNameIt = FunctionIt->GetFName();
			if (AllChildFunctions.Contains(FunctionNameIt)) // is child not super function
			{
				if (FunctionNameIt == CustomPropertyInternal.PropertyValue)
				{
					bValidCustomProperty = true;
				}

				FoundList.Emplace(MoveTemp(FunctionNameIt));
			}
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
		SearchableComboBoxValuesInternal.Add(MakeShareable(new FString(ItemData.ToString())));
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
	const UFunction* TemplateFunction = TemplateFunctionInternal.Get();
	if (!TemplateFunction)
	{
		return false;
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
	TFieldIterator<FProperty> IteratorA(TemplateFunction);
	TFieldIterator<FProperty> IteratorB(Function);

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
void FSettingsFunctionCustomization::InitTemplateFunction(const TSharedRef<IPropertyHandle>& ParentPropertyHandle)
{
	if (TemplateFunctionInternal != nullptr)
	{
		// Set once
		return;
	}

	// Determine current property is wrapped by getter, setter or object context
	FName TemplateFunctionName = NAME_None;
	const FProperty* ParentProperty = ParentPropertyHandle/*ref*/->GetProperty();
	const FName ParentPropertyName = ParentProperty ? ParentProperty->GetFName() : NAME_None;

	static const FName SetterName = GET_MEMBER_NAME_CHECKED(FSettingsDataBase, Setter);
	static const FName GetterName = GET_MEMBER_NAME_CHECKED(FSettingsDataBase, Getter);
	static const FName ContextName = GET_MEMBER_NAME_CHECKED(FSettingsDataBase, ObjectContext);

	if (ParentPropertyName == SetterName)
	{
		static const FName SettingsSetterName = "OnSetter__DelegateSignature";
		TemplateFunctionName = SettingsSetterName;
	}
	else if (ParentPropertyName == GetterName)
	{
		static const FName SettingsGetterName = "OnGetter__DelegateSignature";
		TemplateFunctionName = SettingsGetterName;
	}
	else if (ParentPropertyName == ContextName)
	{
		static const FName SettingsObjectContextName = "OnObjectContext__DelegateSignature";
		TemplateFunctionName = SettingsObjectContextName;
		bIsStaticFunctionInternal = true;
	}

	// Set TemplateFunctionInternal to filter by its signature
	if (!TemplateFunctionName.IsNone())
	{
		const UObject* GameUserSettings = GEngine ? (UObject*)GEngine->GetGameUserSettings() : nullptr;
		const UClass* ScopeClass = GameUserSettings ? GameUserSettings->GetClass() : nullptr;
		TemplateFunctionInternal = ScopeClass ? ScopeClass->FindFunctionByName(TemplateFunctionName, EIncludeSuperFlag::ExcludeSuper) : nullptr;
		ensureMsgf(TemplateFunctionInternal.IsValid(), TEXT("ASSERT: 'TemplateFunctionInternal' is not found"));
	}
}
