// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"
//---
#include "EnhancedActionKeyMapping.h"
//---
#include "InputButtonWidget.generated.h"

/* Widgets hierarchy:
 *
 * ╔UInputControlsWidget
 * ╚════╦UInputCategoryWidget
 *		╚════UInputButtonWidget
 */

/**
 * Input Key Selector wrapper, represents each input button to remap.
 */
UCLASS()
class BOMBER_API UInputButtonWidget final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Sets this button to let player remap input specified in mappable data. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void InitButton(const FEnhancedActionKeyMapping& InMappableData, const class UMyInputMappingContext* InInputMappingContext);

	/** Returns last selected key for the current input selector. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FKey& GetCurrentKey() const { return MappableDataInternal.Key; }

	/** Sets specified key for the current input key selector. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCurrentKey(const FKey& NewKey);

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** A widget for remapping a single key. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UInputKeySelector> InputKeySelector = nullptr;

	/** Contains mappable data for this button. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mappable Data"))
	FEnhancedActionKeyMapping MappableDataInternal;

	/** An input context that is owns this input button. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Input Context"))
	TObjectPtr<const UMyInputMappingContext> InputContextInternal = nullptr;

	/** ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called after the underlying slate widget is constructed. */
	virtual void NativeConstruct() override;

	/** Sets the style for this button. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateStyle();

	/** Called whenever a new key is selected by the user. */
	UFUNCTION()
	void OnKeySelected(FInputChord SelectedKey);

	/** Called whenever the key selection mode starts or stops. */
	UFUNCTION()
	void OnIsSelectingKeyChanged();
};
