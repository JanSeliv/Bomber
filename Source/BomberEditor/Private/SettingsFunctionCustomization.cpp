// Copyright 2021 Yevhenii Selivanov.

#include "SettingsFunctionCustomization.h"
//---
#include "GameFramework/MyGameUserSettings.h"
//---
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"

// Default constructor
FSettingsFunctionCustomization::FSettingsFunctionCustomization()
{
	CustomPropertyNameInternal = GET_MEMBER_NAME_CHECKED(FSettingsFunction, FunctionName);
}

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FSettingsFunctionCustomization::MakeInstance()
{
	return MakeShareable(new FSettingsFunctionCustomization());
}

// Called when the header of the property (the row in the details panel where the property is shown)
void FSettingsFunctionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeHeader(PropertyHandle, HeaderRow, CustomizationUtils);
}

// Called when the children of the property should be customized or extra rows added.
void FSettingsFunctionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Determine current property has to have template as getter or setter
	FName TemplateFunctionName = NAME_None;
	const FProperty* CurrentProperty = PropertyHandle/*ref*/->GetProperty();
	const FName CurrentPropertyName = CurrentProperty ? CurrentProperty->GetFName() : NAME_None;
	if (CurrentPropertyName == GET_MEMBER_NAME_CHECKED(FSettingsRow, Setter))
	{
		static const FName SettingsSetterName = GET_FUNCTION_NAME_CHECKED(UMyGameUserSettings, SetOption);
		TemplateFunctionName = SettingsSetterName;
	}
	else if (CurrentPropertyName == GET_MEMBER_NAME_CHECKED(FSettingsRow, Getter))
	{
		static const FName SettingsGetterName = GET_FUNCTION_NAME_CHECKED(UMyGameUserSettings, GetOption);
		TemplateFunctionName = SettingsGetterName;
	}
	else if (CurrentPropertyName == GET_MEMBER_NAME_CHECKED(FSettingsRow, ObjectContext))
	{
		static const FName SettingsObjectContextName = GET_FUNCTION_NAME_CHECKED(UMyGameUserSettings, GetObjectContext);
		TemplateFunctionName = SettingsObjectContextName;
		bIsStaticFunction = true;
	}

	// Set TemplateFunctionInternal
	if (ensureMsgf(!TemplateFunctionName.IsNone(), TEXT("ASSERT: 'TemplateFunctionName' is none")))
	{
		const UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
		const UClass* ScopeClass = GameUserSettings ? GameUserSettings->GetClass() : nullptr;
		TemplateFunctionInternal = ScopeClass ? ScopeClass->FindFunctionByName(TemplateFunctionName, EIncludeSuperFlag::ExcludeSuper) : nullptr;
	}

	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
}

// Is called for each property on building its row
void FSettingsFunctionCustomization::OnCustomizeChildren(TSharedRef<IPropertyHandle> ChildPropertyHandle, IDetailChildrenBuilder& ChildBuilder, FName PropertyName)
{
	static const FName ClassName = GET_MEMBER_NAME_CHECKED(FSettingsFunction, FunctionClass);
	if (PropertyName == ClassName)
	{
		FunctionClassHandleInternal = ChildPropertyHandle;
	}

	Super::OnCustomizeChildren(ChildPropertyHandle, ChildBuilder, PropertyName);
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

	// Compare with cached value, if equal, then skip refreshing
	const FName ChosenFunctionClassName = ChosenFunctionClass->GetFName();
	if (ChosenFunctionClassName == CachedFunctionClassInternal)
	{
		return;
	}
	CachedFunctionClassInternal = ChosenFunctionClassName;

	SetCustomPropertyEnabled(true);

	SearchableComboBoxValuesInternal.Empty();

	// Add an empty row, so the user can clear the selection if they want
	static const FString NoneName{FName(NAME_None).ToString()};
	const TSharedPtr<FString> NoneNamePtr = MakeShareable(new FString(NoneName));
	SearchableComboBoxValuesInternal.Add(NoneNamePtr);

	TArray<FName> FoundList;
	TArray<FName> AllChildFunctions;
	bool bValidCustomProperty = false;
	const FName CustomPropertyContent(GetCustomPropertyDisplayText().ToString());
	ChosenFunctionClass->GenerateFunctionList(AllChildFunctions);
	for (TFieldIterator<UFunction> It(ChosenFunctionClass); It; ++It)
	{
		const UFunction* FunctionIt = *It;
		if (FunctionIt
		    && FunctionIt != TemplateFunction
		    && (!bIsStaticFunction || FunctionIt->FunctionFlags & FUNC_Static) // only static functions if specified
		    && IsSignatureCompatible(FunctionIt))
		{
			static const FString DelegateSignatureSubStr = "DelegateSignature";
			FName FunctionNameIt = FunctionIt->GetFName();
			if (!FunctionNameIt.ToString().Contains(DelegateSignatureSubStr) // is not delegate
			    && AllChildFunctions.Contains(FunctionNameIt))               // is child not super function
			{
				if (FunctionNameIt == CustomPropertyContent)
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

	CachedFunctionClassInternal = NAME_None;
}

// Returns true if changing custom property currently is not forbidden
bool FSettingsFunctionCustomization::IsAllowedEnableCustomProperty() const
{
	return !CachedFunctionClassInternal.IsNone();
}

// Is called on changing value of any child property
void FSettingsFunctionCustomization::OnAnyChildPropertyChanged()
{
	Super::OnAnyChildPropertyChanged();

	RefreshCustomProperty();
}

// Returns the currently chosen class of functions to display
const UClass* FSettingsFunctionCustomization::GetChosenFunctionClass() const
{
	const IPropertyHandle* ChildHandleIt = FunctionClassHandleInternal.Get();
	const FProperty* ChildProperty = ChildHandleIt ? ChildHandleIt->GetProperty() : nullptr;
	const auto ObjectProperty = ChildProperty ? CastField<FObjectProperty>(ChildProperty) : nullptr;
	const uint8* Data = ObjectProperty ? ChildHandleIt->GetValueBaseAddress((uint8*)MyPropertyOuterInternal.Get()) : nullptr;
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
