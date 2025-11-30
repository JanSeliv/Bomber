// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"

#include "BmrInputCategoryWidget.generated.h"

/* Widgets hierarchy:
 *
 * ╔UBmrInputControlsWidget
 * ╚════╦UBmrInputCategoryWidget
 *		╚════UBmrInputButtonWidget
 */

/**
 * The data structure that holds the information about the input category.
 */
USTRUCT(BlueprintType)
struct FInputCategoryData
{
	GENERATED_BODY()

	/** The name of the input category. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	FText CategoryName = TEXT_NONE;

	/** The input context that contains mappings of this input category data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "[Bomber]")
	TObjectPtr<const class UBmrInputMappingContext> InputMappingContext = nullptr;

	/** All mappings with this input category name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	TArray<struct FPlayerKeyMapping> Mappings;

	/** Returns all categories from the specified input mapping context. */
	static void GetCategoriesDataFromMappings(const UObject& WorldContext, const UBmrInputMappingContext& InInputMappingContext, TArray<FInputCategoryData>& OutInputCategoriesData);
};

/**
 * Contains inputs for along own input context.
 */
UCLASS()
class BOMBER_API UBmrInputCategoryWidget final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Sets the input context to be represented by this widget. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void CreateInputButtons(const FInputCategoryData& InInputCategoryData);

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Is parent widget of all dynamically created buttons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UVerticalBox> VerticalBoxInputButtons = nullptr;

	/** The class of the Input Button Widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "[Bomber]", meta = (BlueprintProtected))
	TSubclassOf<class UBmrInputButtonWidget> InputButtonClass = nullptr;

	/** All dynamically created input button for each mappable input in own Input Context. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<TObjectPtr<class UBmrInputButtonWidget>> InputButtons;

	/** Owned input context that is represented by this widget. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	FInputCategoryData InputCategoryData;

	/** ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called after the underlying slate widget is constructed. */
	virtual void NativeConstruct() override;

	/** Sets the style for this input category. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void UpdateStyle();

	/** Adds all input buttons to the root of this widget. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void AttachInputButtons();
};
