// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
#include "Structures/SettingsRow.h"
//---
#include "SettingsWidget.generated.h"

/**
 * The UI widget of settings.
 */
UCLASS(Abstract)
class BOMBER_API USettingsWidget final : public UUserWidget
{
	GENERATED_BODY()
public:
	/** Returns the amount of settings rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetSettingsTableRowsNum() const { return SettingsTableRowsInternal.Num(); }

	/** Returns the found row by specified tag.
	* @param TagName The key by which the row will be find.
	* @see UMyGameUserSettings::SettingsTableRowsInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FSettingsPicker FindSettingsTableRow(FName TagName) const;

	/**
	 * Returns the object of chosen option.
	 * @param TagName The tag of the option.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	UObject* GetObjectContext(FName TagName) const;

	/* ---------------------------------------------------
	*		Option setters
	* --------------------------------------------------- */

	/**
	 * Set value to the option.
	 * @param TagName The tag of the option.
	 * @param InValue The value to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetOption(FName TagName, int32 InValue);

	/* ---------------------------------------------------
	*		Option getters
	* --------------------------------------------------- */

	/**
	 * Return the value of the option.
	 * @param TagName The tag of the option.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	int32 GetOption(FName TagName) const;

protected:
	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ConstructSettings();

	/* ---------------------------------------------------
	*		Option getters
	* --------------------------------------------------- */

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddButton(const FSettingsPrimary& Primary, const FSettingsButton& Data);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCheckbox(const FSettingsPrimary& Primary, const FSettingsCheckbox& Data);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCombobox(const FSettingsPrimary& Primary, const FSettingsCombobox& Data);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddSlider(const FSettingsPrimary& Primary, const FSettingsSlider& Data);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddTextSimple(const FSettingsPrimary& Primary, const FSettingsTextSimple& Data);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddTextInput(const FSettingsPrimary& Primary, const FSettingsTextInput& Data);

	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Contains all settings. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Table Rows", ShowOnlyInnerProperties))
	TMap<FName, FSettingsPicker> SettingsTableRowsInternal; //[D]
};
