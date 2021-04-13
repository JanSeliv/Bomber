// Copyright 2021 Yevhenii Selivanov.

#include "MorphDataCustomization.h"
//---
#include "AnimNotifyState_PlayMorph.h"

typedef FMorphDataCustomization ThisClass;

// The name of class to be customized
const FName FMorphDataCustomization::PropertyClassName = FMorphData::StaticStruct()->GetFName();

// Default constructor
FMorphDataCustomization::FMorphDataCustomization()
{
	CustomProperty.PropertyName = GET_MEMBER_NAME_CHECKED(FMorphData, Morph);
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

// Is called for each property on building its row
void FMorphDataCustomization::OnCustomizeChildren(IDetailChildrenBuilder& ChildBuilder, const FPropertyData& PropertyData)
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

	SearchableComboBoxValuesInternal.Empty();
	SearchableComboBoxValuesInternal.Reserve(PreviewMesh->MorphTargetIndexMap.Num() + 1);

	// Add an empty row, so the user can clear the selection if they want
	static const FString NoneName{FName(NAME_None).ToString()};
	const TSharedPtr<FString> NoneNamePtr = MakeShareable(new FString(NoneName));
	SearchableComboBoxValuesInternal.Add(NoneNamePtr);

	for (const auto& MorphTargetIt : PreviewMesh->MorphTargetIndexMap)
	{
		// Add this to the searchable text box as an FString so users can type and find it
		const FString MorphTarget = MorphTargetIt.Key.ToString();
		SearchableComboBoxValuesInternal.Add(MakeShareable(new FString(MorphTarget)));
	}

	// Will refresh searchable combo box
	Super::RefreshCustomProperty();
}
