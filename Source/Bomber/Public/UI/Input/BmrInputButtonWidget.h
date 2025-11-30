// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"

// UE
#include "UserSettings/EnhancedInputUserSettings.h" // FPlayerKeyMapping

#include "BmrInputButtonWidget.generated.h"

/* Widgets hierarchy:
 *
 * ╔UInputControlsWidget
 * ╚════╦UBmrInputCategoryWidget
 *		╚════UBmrInputButtonWidget
 */

/**
 * Input Key Selector wrapper, represents each input button to remap.
 */
UCLASS()
class BOMBER_API UBmrInputButtonWidget final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Sets this button to let player remap input specified in mappable data. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void InitButton(const FPlayerKeyMapping& InMappableData, const class UBmrInputMappingContext* InInputMappingContext);

	/** Returns last selected key for the current input selector. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FKey& GetCurrentKey() const { return MappableData.GetCurrentKey(); }

	/** Sets specified key for the current input key selector. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetCurrentKey(const FKey& NewKey);

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** A widget for remapping a single key. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UInputKeySelector> InputKeySelector = nullptr;

	/** Contains mappable data for this button. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	FPlayerKeyMapping MappableData;

	/** An input context that is owns this input button. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<const UBmrInputMappingContext> InputContext = nullptr;

	/** ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called after the underlying slate widget is constructed. */
	virtual void NativeConstruct() override;

	/** Sets the style for this button. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void UpdateStyle();

	/** Called whenever a new key is selected by the user. */
	UFUNCTION()
	void OnKeySelected(FInputChord SelectedKey);

	/** Called whenever the key selection mode starts or stops. */
	UFUNCTION()
	void OnIsSelectingKeyChanged();
};
