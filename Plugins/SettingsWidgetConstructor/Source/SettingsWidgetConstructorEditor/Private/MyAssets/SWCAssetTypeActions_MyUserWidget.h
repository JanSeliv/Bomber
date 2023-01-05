// Copyright (c) Yevhenii Selivanov

#pragma once

#include "AssetTypeActions_Base.h"
#include "WidgetBlueprint.h"
#include "Factories/Factory.h"
//---
#include "SWCAssetTypeActions_MyUserWidget.generated.h"

/**
 * The widget blueprint enables extending custom user widget.
 */
UCLASS()
class USWCMyUserWidgetBlueprint : public UWidgetBlueprint
{
	GENERATED_BODY()

public:
	virtual bool AllowEditorWidget() const override { return true; }
	virtual void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
};

/**
 * Is responsible for creating new custom user widget asset in the 'Add' context menu.
 * Basically, is taken from FAssetTypeActions_WidgetBlueprint since we can't derive to private include
 * Is abstract to ignore this factory by !CurrentClass->HasAnyClassFlags(CLASS_Abstract),
 * child should not be abstracted
 */
UCLASS(Abstract)
class USWCMyUserWidgetFactory : public UFactory
{
	GENERATED_BODY()

public:
	USWCMyUserWidgetFactory();

#pragma region OverrideInChild
	virtual FText GetDisplayName() const override;
	virtual TSubclassOf<class UUserWidget> GetWidgetClass() const;
#pragma endregion OverrideInChild

	virtual bool CanCreateNew() const override { return true; }
	virtual bool ShouldShowInNewMenu() const override { return true; }

	/** Is copied from UWidgetBlueprintFactory since it's private. */
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

/**
 * Shows additional actions in the Context Menu of the custom user widget.
 * Basically, is taken from FAssetTypeActions_WidgetBlueprint since we can't derive to private include
 */
class FAssetTypeActions_MyUserWidget : public FAssetTypeActions_Base
{
public:
	virtual ~FAssetTypeActions_MyUserWidget() override = default;

#pragma region OverrideInChild
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override { return FColor::Blue; }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::UI; }
#pragma endregion OverrideInChild

	/** Does not require to be overridden since UMyUserWidgetFactory::GetWidgetClass() is known by UMyUserWidgetBlueprint that is specified here. */
	virtual UClass* GetSupportedClass() const override { return USWCMyUserWidgetBlueprint::StaticClass(); }

	/** Is copied from FAssetTypeActions_WidgetBlueprint since it's private. */
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
};
