// Copyright 2021 Yevhenii Selivanov.

#include "AttachedMeshCustomization.h"
//---
#include "LevelActors/PlayerCharacter.h"
//---
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "LevelEditor.h"
#include "PropertyCustomizationHelpers.h"
#include "SceneOutliner/Private/SSocketChooser.h"
#include "Toolkits/ToolkitManager.h"

typedef FAttachedMeshCustomization ThisClass;

// The name of class to be customized
const FName FAttachedMeshCustomization::PropertyClassName = FAttachedMesh::StaticStruct()->GetFName();

// Default constructor
FAttachedMeshCustomization::FAttachedMeshCustomization()
{
	CustomPropertyNameInternal = GET_MEMBER_NAME_CHECKED(FAttachedMesh, Socket);
}

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FAttachedMeshCustomization::MakeInstance()
{
	return MakeShareable(new ThisClass());
}

// Called when the header of the property (the row in the details panel where the property is shown)
void FAttachedMeshCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeHeader(PropertyHandle, HeaderRow, CustomizationUtils);
}

// Called when the children of the property should be customized or extra rows added.
void FAttachedMeshCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Super::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
}

// Is called for each property on building its row
void FAttachedMeshCustomization::OnCustomizeChildren(TSharedRef<IPropertyHandle> ChildPropertyHandle, IDetailChildrenBuilder& ChildBuilder, FName PropertyName)
{
	Super::OnCustomizeChildren(ChildPropertyHandle, ChildBuilder, PropertyName);
}

// Is called on adding the custom property
void FAttachedMeshCustomization::AddCustomPropertyRow(const FText& PropertyDisplayText, IDetailChildrenBuilder& ChildBuilder)
{
	ChildBuilder.AddCustomRow(PropertyDisplayText)
	            .NameContent()
		[
			SNew(STextBlock)
        .Text(PropertyDisplayText)
        .Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SEditableTextBox)
            .Text(this, &ThisClass::GetCustomPropertyDisplayText)
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
					FSimpleDelegate::CreateSP(this, &ThisClass::OnBrowseSocket),
					FText::FromString("Select a different Parent Socket - cannot change socket on inherited components"),
					TAttribute<bool>(true)
					)
			]
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .HAlign(HAlign_Center)
			  .VAlign(VAlign_Center)
			  .Padding(2.0f, 1.0f)
			[
				PropertyCustomizationHelpers::MakeClearButton(
					FSimpleDelegate::CreateSP(this, &ThisClass::InvalidateCustomProperty),
					FText::FromString("Clear the Parent Socket - cannot change socket on inherited components"),
					TAttribute<bool>(true)
					)
			]
		];
}

// Push menu to allow user choose socket
void FAttachedMeshCustomization::OnBrowseSocket()
{
	const UObject* PlayerRowOuter = MyPropertyOuterInternal.Get();
	if (!PlayerRowOuter)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnBrowseSocket - FAIL - Can not obtain UPlayerRow data"));
		return;
	}

	// Get mesh
	USkeletalMesh* SkeletalMeshAsset = nullptr;
	static const FName ParentMeshPropertyName = GET_MEMBER_NAME_CHECKED(UPlayerRow, Mesh);
	const UClass* PlayerRowClass = PlayerRowOuter->GetClass();
	if (const auto ParentMeshProperty = CastField<FObjectProperty>(PlayerRowClass->FindPropertyByName(ParentMeshPropertyName)))
	{
		SkeletalMeshAsset = Cast<USkeletalMesh>(ParentMeshProperty->GetPropertyValue_InContainer(PlayerRowOuter));
	}
	if (!SkeletalMeshAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnBrowseSocket - FAIL - the parent skeletal mesh is null"));
		return;
	}

	// Create component if not created yet
	USkeletalMeshComponent* ParentMeshComponent = ParentMeshCompInternal.Get();
	if (!ParentMeshComponent)
	{
		ParentMeshComponent = NewObject<USkeletalMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
		ParentMeshCompInternal = ParentMeshComponent;
	}

	// Attach mesh and check sockets
	ParentMeshComponent->SetSkeletalMesh(SkeletalMeshAsset);
	if (!ParentMeshComponent->HasAnySockets())
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnBrowseSocket - FAIL - there are no sockets on the mesh ---"));
		return;
	}

	// Find editor to get parent widget to attach menu
	const TSharedPtr<IToolkit> ToolkitEditor = FToolkitManager::Get().FindEditorForAsset(PlayerRowOuter->GetOuter());
	if (!ensureMsgf(ToolkitEditor, TEXT("--- OnBrowseSocket - FAIL - the Editor was not found ---")))
	{
		return;
	}

	// Pop up a combo box to pick a socket or bone from mesh
	FSlateApplication::Get().PushMenu(
		ToolkitEditor->GetToolkitHost()->/*ref*/GetParentWidget(),
		FWidgetPath(),
		SNew(SSocketChooserPopup)
        .SceneComponent(ParentMeshComponent)
        .OnSocketChosen(this, &Super::SetCustomPropertyValue),
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);
}
