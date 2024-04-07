// Copyright (c) Yevhenii Selivanov

#pragma once

#include "View/MVVMViewModelContextResolver.h"
//---
#include "MVVMMyContextResolver.generated.h"

/**
 * This resolver automatically creates and destroys the View Models of given Views.
 * The main feature of this Resolver is calling extra events that original View Model does not have.
 * For instance, 'OnViewModelConstruct' event that allows a View to bind itself to different delegates in order to update own data.
 * To make it work:
 * 1. Widget has to have 'Creation Type' selected as 'Resolver' with this class.
 * 2. View Model has to be a subclass of 'UMVVM_MyBaseViewModel'.
 */
UCLASS(Blueprintable, BlueprintType, DisplayName = "My Context Resolver")
class MYUTILS_API UMVVMMyContextResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

protected:
	/** Is called to create a new instance of the ViewModel.
	 *  To make it call, a Widget has to have 'Creation Type' selected as 'Resolver' with this class. */
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;

	/** Is called to destroy the instance of the ViewModel. */
	virtual void DestroyInstance(const UObject* ViewModel, const UMVVMView* View) const override;
};
