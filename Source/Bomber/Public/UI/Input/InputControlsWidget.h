// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"
//---
#include "InputCoreTypes.h"
//---
#include "InputControlsWidget.generated.h"

/* Widgets hierarchy:
 *
 * ╔UInputControlsWidget
 * ╚════╦UInputCategoryWidget
 *		╚════UInputButtonWidget
 */

/**
 * Allows player to rebind input mappings.
 */
UCLASS()
class BOMBER_API UInputControlsWidget final : public USettingCustomWidget
{
	GENERATED_BODY()

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Is parent widget of all dynamically created categories. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UScrollBox> ScrollBoxInputCategories = nullptr;

	/** The class of the Input Button Widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Input Category Class"))
	TSubclassOf<class UInputCategoryWidget> InputCategoryClassInternal = nullptr;

	/** Contains all dynamically created categories, where every category represents own mapping context. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Input Categories"))
	TArray<TObjectPtr<class UInputCategoryWidget>> InputCategoriesInternal;

	/** ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/** Adds input categories for each mapping context. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void CreateAllInputCategories();
};
