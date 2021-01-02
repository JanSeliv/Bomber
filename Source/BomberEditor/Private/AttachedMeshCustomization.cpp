// Copyright 2021 Yevhenii Selivanov.

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

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FAttachedMeshCustomization::MakeInstance()
{
	return MakeShareable(new FAttachedMeshCustomization());
}

// Called when the header of the property (the row in the details panel where the property is shown)
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
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);
	PlayerRowOuterInternal = OuterObjects.IsValidIndex(0) ? OuterObjects[0] : nullptr;

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

		// Customize the FAttachedMesh::Socket
		static const FName SocketPropertyName = GET_MEMBER_NAME_CHECKED(FAttachedMesh, Socket);
		if (SocketPropertyName == ChildProperty->GetFName())
		{
			if (const FNameProperty* NameProperty = CastField<FNameProperty>(ChildProperty))
			{
				SocketPropertyHandleInternal = ChildHandleIt;
				AddSocketWidgetRow(ChildBuilder);
			}
		}
		else
		{
			// Add each another property to the Details Panel without customization
			ChildBuilder.AddProperty(ChildHandleIt);
		}
	}
}

// Customize a Socket property, will add the chosen text row, the Select and Clear buttons
void FAttachedMeshCustomization::AddSocketWidgetRow(IDetailChildrenBuilder& ChildBuilder)
{
	IPropertyHandle* SocketHandle = SocketPropertyHandleInternal.Get();
	if (!SocketHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- GetSocketName - FAIL - Can not obtain SocketProperty data"));
		return;
	}

	ChildBuilder.AddCustomRow(SocketHandle->GetPropertyDisplayName())
	            .NameContent()
		[
			SNew(STextBlock)
			.Text(SocketHandle->GetPropertyDisplayName())
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SEditableTextBox)
				.Text(this, &FAttachedMeshCustomization::GetSocketFromProperty)
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
					FSimpleDelegate::CreateSP(this, &FAttachedMeshCustomization::OnClearSocket),
					FText::FromString("Clear the Parent Socket - cannot change socket on inherited components"),
					TAttribute<bool>(true)
					)
			]
		];
}

// Push menu to allow user choose socket
void FAttachedMeshCustomization::OnBrowseSocket()
{
	const UObject* PlayerRowOuter = PlayerRowOuterInternal.Get();
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
		ToolkitEditor->/*ref*/GetToolkitHost()->/*ref*/GetParentWidget(),
		FWidgetPath(),
		SNew(SSocketChooserPopup)
        .SceneComponent(ParentMeshComponent)
        .OnSocketChosen(this, &FAttachedMeshCustomization::SetSocketToProperty),
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);
}

// Set chosen socket to None
void FAttachedMeshCustomization::OnClearSocket()
{
	SetSocketToProperty(NAME_None);
}

// Executed every tick
FText FAttachedMeshCustomization::GetSocketFromProperty() const
{
	FText Value(FText::GetEmpty());
	SocketPropertyHandleInternal->GetValueAsDisplayText(Value);
	return Value;
}

// Executed on socket selection
void FAttachedMeshCustomization::SetSocketToProperty(FName BoneName)
{
	IPropertyHandle* SocketPropertyHandle = SocketPropertyHandleInternal.Get();
	if (!SocketPropertyHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- SetSocketToProperty - FAIL - Can not obtain SocketPropertyHandle data"));
		return;
	}

	SocketPropertyHandle->SetValueFromFormattedString(BoneName.ToString());
}
