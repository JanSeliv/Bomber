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
UCLASS(Abstract, Blueprintable, BlueprintType, DisplayName = "[Abstract] My Base View Model")
class MYUTILS_API UMVVM_MyBaseViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** Returns the world where this View Model is created. */
	virtual UWorld* GetWorld() const override;

	/** If false, the View Model will not be constructed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "C++")
	bool CanConstructViewModel() const;
	virtual bool CanConstructViewModel_Implementation() const;

	/** Is called when this View Model is constructed.
	 * Is used for bindings to the changes in other systems in order to update own data. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++")
	void OnViewModelConstruct(const class UUserWidget* UserWidget);
	virtual void OnViewModelConstruct_Implementation(const class UUserWidget* UserWidget) {}

	/** Is called when this View Model is destructed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++")
	void OnViewModelDestruct();
	virtual void OnViewModelDestruct_Implementation() {}
};
