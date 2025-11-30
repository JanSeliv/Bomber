// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"

#include "BmrInputControlsWidget.generated.h"

/* Widgets hierarchy:
 *
 * ╔UBmrInputControlsWidget
 * ╚════╦UBmrInputCategoryWidget
 *		╚════UBmrInputButtonWidget
 */

/**
 * Allows player to rebind input mappings.
 */
UCLASS()
class BOMBER_API UBmrInputControlsWidget final : public USettingCustomWidget
{
	GENERATED_BODY()

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Is parent widget of all dynamically created categories. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UScrollBox> ScrollBoxInputCategories = nullptr;

	/** The class of the Input Button Widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "[Bomber]", meta = (BlueprintProtected))
	TSubclassOf<class UBmrInputCategoryWidget> InputCategoryClass = nullptr;

	/** Contains all dynamically created categories, where every category represents own mapping context. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<TObjectPtr<class UBmrInputCategoryWidget>> InputCategories;

	/** ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/** Adds input categories for each mapping context. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void CreateAllInputCategories();
};
