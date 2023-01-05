// Copyright (c) Yevhenii Selivanov.

#include "MorphDataCustomization.h"
//---
#include "MorphsPlayerTypes.h"
//---
#include "Animation/AnimNotifies/AnimNotifyState.h"

typedef FMorphDataCustomization ThisClass;

// The name of class to be customized
const FName FMorphDataCustomization::PropertyClassName = FMorphData::StaticStruct()->GetFName();

// Default constructor
FMorphDataCustomization::FMorphDataCustomization()
{
	CustomPropertyInternal.PropertyName = GET_MEMBER_NAME_CHECKED(FMorphData, Morph);
}

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FMorphDataCustomization::MakeInstance()
{
	return MakeShareable(new ThisClass());
}

// Called when the header of the property (the row in the details panel where the property is shown)
void FMorphDataCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Do not use the header panel at all
}

// Called when the children of the property should be customized or extra rows added.
void FMorphDataCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
}

// Creates customization for the Morph Data
void FMorphDataCustomization::RegisterMorphDataCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Allows to choose a morph
	PropertyModule.RegisterCustomPropertyTypeLayout(
		PropertyClassName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMorphDataCustomization::MakeInstance)
		);

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the Morph Data
void FMorphDataCustomization::UnregisterMorphDataCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}

// Is called for each property on building its row
void FMorphDataCustomization::OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, FPropertyData& PropertyData)
{
	Super::OnCustomizeChildren(ChildBuilder, PropertyData);
}

// Is called on adding the custom property
void FMorphDataCustomization::AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder)
{
	// Super will add the searchable combo box
	Super::AddCustomPropertyRow(PropertyDisplayText, ChildBuilder);
}

// Set new values for the list of selectable members
void FMorphDataCustomization::RefreshCustomProperty()
{
	const auto AnimNotifyState = Cast<UAnimNotifyState>(MyPropertyOuterInternal.Get());
	const auto AnimSequence = AnimNotifyState ? Cast<UAnimSequence>(AnimNotifyState->GetOuter()) : nullptr;
	const USkeletalMesh* PreviewMesh = AnimSequence ? AnimSequence->GetPreviewMesh() : nullptr;
	const FName MeshName = PreviewMesh ? PreviewMesh->GetFName() : NAME_None;
	if (MeshName.IsNone()
	    || MeshName == CachedMeshNameInternal)
	{
		return;
	}
	CachedMeshNameInternal = MeshName;

	ResetSearchableComboBox();
	const TMap<FName, int32>& MorphTargetIndexMap = PreviewMesh->GetMorphTargetIndexMap();
	SearchableComboBoxValuesInternal.Reserve(MorphTargetIndexMap.Num() + 1);

	for (const TTuple<FName, int32>& MorphTargetIt : MorphTargetIndexMap)
	{
		// Add this to the searchable text box as an FString so users can type and find it
		const FString MorphTarget = MorphTargetIt.Key.ToString();
		SearchableComboBoxValuesInternal.Emplace(MakeShareable(new FString(MorphTarget)));
	}

	// Will refresh searchable combo box
	Super::RefreshCustomProperty();
}
