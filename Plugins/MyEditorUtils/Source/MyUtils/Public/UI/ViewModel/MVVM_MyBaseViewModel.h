// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MVVMViewModelBase.h"
//---
#include "MVVM_MyBaseViewModel.generated.h"

/**
 * Base class for all View Model (MVVM) views.
 * Is connected to the widget to provide access UI data.
 * Widget has to have 'Creation Type' selected as 'Resolver' with 'MVVM_ByBaseContextResolved' class.
 */
UCLASS(Blueprintable, BlueprintType, DisplayName = "My Base View Model")
class MYUTILS_API UMVVM_MyBaseViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** Is called when this View Model is constructed.
	 * Is used for bindings to the changes in other systems in order to update own data. */
	UFUNCTION(BlueprintCallable, Category = "C++", BlueprintNativeEvent)
	void OnViewModelConstruct(const class UUserWidget* UserWidget);
	virtual void OnViewModelConstruct_Implementation(const class UUserWidget* UserWidget) {}

	/** Is called when this View Model is destructed. */
	UFUNCTION(BlueprintCallable, Category = "C++", BlueprintNativeEvent)
	void OnViewModelDestruct();
	virtual void OnViewModelDestruct_Implementation() {}
};
