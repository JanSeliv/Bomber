// Copyright 2020 Yevhenii Selivanov.

#include "AttachedMeshCustomization.h"
//---
#include "LevelActors/PlayerCharacter.h"
//---
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "LevelEditor.h"
#include "PropertyCustomizationHelpers.h"
#include "SceneOutliner/Private/SSocketChooser.h"
#include "Toolkits/ToolkitManager.h"

// Makes a new instance of this detail layout class for a specific detail view requesting it.
TSharedRef<IPropertyTypeCustomization> FAttachedMeshCustomization::MakeInstance()
{
	return MakeShareable(new FAttachedMeshCustomization());
}

void FAttachedMeshCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
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
void FAttachedMeshCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Store property and object to the UPlayerRow to get its parent mesh
	SharedPropertyHandle = PropertyHandle;
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);
	WeakPlayerRowOuter = OuterObjects.IsValidIndex(0) ? OuterObjects[0] : nullptr;

	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef<IPropertyHandle> ChildHandleIt = PropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		const FProperty* ChildProperty = ChildHandleIt->GetProperty();
		if (!ChildProperty)
		{
			continue;
		}

		// Customize the FAttachedMesh::Bone
		static const FName BonePropertyName = GET_MEMBER_NAME_CHECKED(FAttachedMesh, Bone);
		if (BonePropertyName == ChildProperty->GetFName())
		{
			if (const FNameProperty* NameProperty = CastField<FNameProperty>(ChildProperty))
			{
				ChildBuilder.AddCustomRow(ChildHandleIt->GetPropertyDisplayName())
				            .NameContent()
					[
						SNew(STextBlock)
                            .Text(ChildHandleIt->GetPropertyDisplayName())
                            .Font(IDetailLayoutBuilder::GetDetailFont())
					]
					.ValueContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SEditableTextBox)
                            .Text(this, &FAttachedMeshCustomization::GetSocketName)
                            .IsReadOnly(true)
                            .Font(IDetailLayoutBuilder::GetDetailFont())
						]
						+ SHorizontalBox::Slot()
						  .AutoWidth()
						  .HAlign(HAlign_Center)
						  .VAlign(VAlign_Center)
						  .Padding(2.0f, 1.0f)
						[
							PropertyCustomizationHelpers::MakeBrowseButton(
								FSimpleDelegate::CreateSP(this, &FAttachedMeshCustomization::OnBrowseSocket),
								FText::FromString("Select a different Parent Socket - cannot change socket on inherited components"),
								TAttribute<bool>(this, &FAttachedMeshCustomization::CanChangeSocket)
								)
						]
						+ SHorizontalBox::Slot()
						  .AutoWidth()
						  .HAlign(HAlign_Center)
						  .VAlign(VAlign_Center)
						  .Padding(2.0f, 1.0f)
						[
							PropertyCustomizationHelpers::MakeClearButton(
								FSimpleDelegate::CreateSP(this, &FAttachedMeshCustomization::OnClearSocket),
								FText::FromString("Clear the Parent Socket - cannot change socket on inherited components"),
								TAttribute<bool>(this, &FAttachedMeshCustomization::CanChangeSocket)
								)
						]
					];
			}
		}
		else
		{
			// Add each another property to the Details Panel without customization
			ChildBuilder.AddProperty(ChildHandleIt);
		}
	}
}

FText FAttachedMeshCustomization::GetSocketName() const
{
	return FText::FromString("TEST");
}

bool FAttachedMeshCustomization::CanChangeSocket() const
{
	return true;
}

void FAttachedMeshCustomization::OnBrowseSocket()
{
	const IPropertyHandle* PropertyHandle = SharedPropertyHandle.Get();
	UObject* PlayerRow = WeakPlayerRowOuter.Get();
	if (!PropertyHandle
	    || !PlayerRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnBrowseSocket - FAIL - Can not obtain UPlayerRow data"));
		return;
	}

	// Get mesh
	USkeletalMesh* SkeletalMeshAsset = nullptr;
	static const FName ParentMeshPropertyName = GET_MEMBER_NAME_CHECKED(UPlayerRow, Mesh);
	const UClass* PlayerRowClass = PlayerRow->GetClass();
	if (const auto ParentMeshProperty = CastField<FObjectProperty>(PlayerRowClass->FindPropertyByName(ParentMeshPropertyName)))
	{
		SkeletalMeshAsset = Cast<USkeletalMesh>(ParentMeshProperty->GetPropertyValue_InContainer(PlayerRow));
	}
	if (!SkeletalMeshAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnBrowseSocket - FAIL - the parent skeletal mesh is null"));
		return;
	}

	// Create component if not created yet
	USkeletalMeshComponent* ParentMeshComponent = WeakParentMeshComponent.Get();
	if (!ParentMeshComponent)
	{
		ParentMeshComponent = NewObject<USkeletalMeshComponent>(PlayerRow);
		WeakParentMeshComponent = ParentMeshComponent;
	}

	// Attach mesh and check sockets
	ParentMeshComponent->SetSkeletalMesh(SkeletalMeshAsset);
	if (!ParentMeshComponent->HasAnySockets())
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnBrowseSocket - FAIL - there are no sockets on the mesh ---"));
		return;
	}

	// Find editor to get parent widget to attach menu
	const TSharedPtr<IToolkit> ToolkitEditor = FToolkitManager::Get().FindEditorForAsset(PlayerRow->GetOuter());
	if (!ensureMsgf(ToolkitEditor, TEXT("--- OnBrowseSocket - FAIL - the Editor was not found ---")))
	{
		return;
	}

	// Pop up a combo box to pick a socket or bone from mesh
	FSlateApplication::Get().PushMenu(
		ToolkitEditor->GetToolkitHost()->GetParentWidget(),
		FWidgetPath(),
		SNew(SSocketChooserPopup)
        .SceneComponent(ParentMeshComponent)
        .OnSocketChosen(this, &FAttachedMeshCustomization::OnSocketSelection),
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);
}

void FAttachedMeshCustomization::OnClearSocket()
{
}

void FAttachedMeshCustomization::OnSocketSelection(FName SocketName)
{
}
