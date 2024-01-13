// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"
//---
#include "InputCategoryWidget.generated.h"

/* Widgets hierarchy:
 *
 * ╔UInputControlsWidget
 * ╚════╦UInputCategoryWidget
 *		╚════UInputButtonWidget
 */

/**
 * The data structure that holds the information about the input category.
 */
USTRUCT(BlueprintType)
struct FInputCategoryData
{
	GENERATED_BODY()

	/** The name of the input category. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FText CategoryName = TEXT_NONE;

	/** The input context that contains mappings of this input category data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<const class UMyInputMappingContext> InputMappingContext = nullptr;

	/** All mappings with this input category name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TArray<struct FEnhancedActionKeyMapping> Mappings;

	/** Returns all categories from the specified input mapping context. */
	static void GetCategoriesDataFromMappings(const UMyInputMappingContext& InInputMappingContext, TArray<FInputCategoryData>& OutInputCategoriesData);
};

/**
 * Contains inputs for along own input context.
 */
UCLASS()
class BOMBER_API UInputCategoryWidget final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Sets the input context to be represented by this widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void CreateInputButtons(const FInputCategoryData& InInputCategoryData);

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Is parent widget of all dynamically created buttons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UVerticalBox> VerticalBoxInputButtons = nullptr;

	/** The class of the Input Button Widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Input Button Class"))
	TSubclassOf<class UInputButtonWidget> InputButtonClassInternal = nullptr;

	/** All dynamically created input button for each mappable input in own Input Context. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Input Buttons"))
	TArray<TObjectPtr<class UInputButtonWidget>> InputButtonsInternal;

	/** Owned input context that is represented by this widget. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Input Category Data"))
	FInputCategoryData InputCategoryDataInternal;

	/** ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called after the underlying slate widget is constructed. */
	virtual void NativeConstruct() override;

	/** Sets the style for this input category. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateStyle();

	/** Adds all input buttons to the root of this widget. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void AttachInputButtons();
};
