// Copyright 2021 Yevhenii Selivanov.

#include "MyPropertyTypeCustomization.h"
//---
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "SSearchableComboBox.h"

typedef Super ThisClass;

// Called when the header of the property (the row in the details panel where the property is shown)
void FMyPropertyTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Use default the header details panel
	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];
}

// Called when the children of the property should be customized or extra rows added.
void FMyPropertyTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Find outer
	TArray<UObject*> OuterObjects;
	PropertyHandle/*ref*/->GetOuterObjects(OuterObjects);
	MyPropertyOuterInternal = OuterObjects.IsValidIndex(0) ? OuterObjects[0] : nullptr;

	// Bind to delegate
	PropertyHandle/*ref*/->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &ThisClass::RefreshCustomProperty));

	// Customize
	uint32 NumChildren;
	PropertyHandle/*ref*/->GetNumChildren(NumChildren);
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef<IPropertyHandle> ChildPropertyHandleIt = PropertyHandle/*ref*/->GetChildHandle(ChildIndex).ToSharedRef();
		if (const FProperty* ChildProperty = ChildPropertyHandleIt->GetProperty())
		{
			OnCustomizeChildren(ChildPropertyHandleIt, ChildBuilder, ChildProperty->GetFName());
		}
	}
}

// Get chosen displayed value contained in the property to be customized
FText FMyPropertyTypeCustomization::GetCustomPropertyDisplayText() const
{
	FText Value(FText::GetEmpty());
	if (const IPropertyHandle* PropertyHandle = CustomPropertyHandleInternal.Get())
	{
		PropertyHandle->GetValueAsDisplayText(/*Out*/Value);
	}

	return Value;
}

// Set the FName value into the property
void FMyPropertyTypeCustomization::SetCustomPropertyValue(FName Value)
{
	const FString StringToSet = Value.ToString();
	if (CustomPropertyHandleInternal)
	{
		// Set value into property
		CustomPropertyHandleInternal->SetValueFromFormattedString(StringToSet);
	}

	if (const TSharedPtr<STextBlock> RowTextWidget = RowTextWidgetInternal.Pin())
	{
		// Update value on displayed widget
		RowTextWidget->SetText(FText::FromString(StringToSet));
	}
}

// Returns true if custom property is active (not greyed out)
bool FMyPropertyTypeCustomization::IsCustomPropertyEnabled() const
{
	bool bIsEnabled = false;
	if (const TSharedPtr<SSearchableComboBox> SearchableComboBox = SearchableComboBoxInternal.Pin())
	{
		bIsEnabled = SearchableComboBox->IsEnabled();
	}

	return bIsEnabled;
}

// Set true to activate property, false to grey out it (read-only)
void FMyPropertyTypeCustomization::SetCustomPropertyEnabled(bool bEnabled)
{
	const bool bIsAllowedEnableCustomProperty = IsAllowedEnableCustomProperty();
	if (bEnabled && !bIsAllowedEnableCustomProperty)
	{
		// Enable is forbidden
		return;
	}

	if (const TSharedPtr<SSearchableComboBox> SearchableComboBox = SearchableComboBoxInternal.Pin())
	{
		SearchableComboBox->SetEnabled(bEnabled);
	}
}

// Is called for each property on building its row
void FMyPropertyTypeCustomization::OnCustomizeChildren(TSharedRef<IPropertyHandle> ChildPropertyHandle, IDetailChildrenBuilder& ChildBuilder, FName PropertyName)
{
	if (PropertyName != CustomPropertyNameInternal)
	{
		// Add each another property to the Details Panel without customization
		ChildBuilder.AddProperty(ChildPropertyHandle);
		return;
	}

	// Add as searchable combo box by default
	CustomPropertyHandleInternal = ChildPropertyHandle;
	AddCustomPropertyRow(ChildPropertyHandle/*ref*/->GetPropertyDisplayName(), ChildBuilder);
}

// Will add the default searchable combo box
void FMyPropertyTypeCustomization::AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder)
{
	RefreshCustomProperty();

	// Will add the searchable combo box by default
	const TSharedRef<STextBlock> TextRowWidgetRef =
		SNew(STextBlock)
		.Text(GetCustomPropertyDisplayText());
	RowTextWidgetInternal = TextRowWidgetRef;

	const TSharedRef<SSearchableComboBox> SearchableComboBoxRef =
		SNew(SSearchableComboBox)
                .OptionsSource(&SearchableComboBoxValuesInternal)
                .OnGenerateWidget_Lambda([](const TSharedPtr<FString> InItem) -> TSharedRef<SWidget>
		                         {
			                         return SNew(STextBlock).Text(FText::FromString(*InItem));
		                         })
                .OnSelectionChanged(this, &FMyPropertyTypeCustomization::OnCustomPropertyChosen)
                .ContentPadding(2.f)
                .MaxListHeight(200.f)
				.IsEnabled(IsAllowedEnableCustomProperty())
				.Content()
		[
			TextRowWidgetRef
		];
	SearchableComboBoxInternal = SearchableComboBoxRef;

	ChildBuilder.AddCustomRow(PropertyDisplayText)
	            .NameContent()
		[
			SNew(STextBlock)
               .Text(PropertyDisplayText)
               .Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .VAlign(VAlign_Fill)
			  .Padding(0.f)
			[
				SearchableComboBoxRef
			]
		];
}

//Set new values for the list of selectable members
void FMyPropertyTypeCustomization::RefreshCustomProperty()
{
	if (const TSharedPtr<SSearchableComboBox> SearchableComboBox = SearchableComboBoxInternal.Pin())
	{
		SearchableComboBox->RefreshOptions();
	}
}

// Is called to deactivate custom property
void FMyPropertyTypeCustomization::InvalidateCustomProperty()
{
	SetCustomPropertyEnabled(false);

	SetCustomPropertyValue(NAME_None);
}

// Called when the children of the property should be customized or extra rows added
void FMyPropertyTypeCustomization::OnCustomPropertyChosen(TSharedPtr<FString> SelectedStringPtr, ESelectInfo::Type SelectInfo)
{
	if (const FString* SelectedString = SelectedStringPtr.Get())
	{
		SetCustomPropertyValue(**SelectedString);
	}
}
