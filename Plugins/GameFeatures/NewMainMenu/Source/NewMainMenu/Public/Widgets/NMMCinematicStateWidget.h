// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

#include "NMMCinematicStateWidget.generated.h"

/**
 * Is active while game is in cinematic state, is responsible for skipping cinematic.
 */
UCLASS()
class NEWMAINMENU_API UNMMCinematicStateWidget : public UUserWidget
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Applies the given time to hold the skip progress to skip the cinematic. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetCurrentHoldTime(float NewHoldTime);

	/** Reset to default state. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void ResetWidget();

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** The button to skip current cinematic. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> SkipCinematicButton = nullptr;

	/** Represents the progress of the hold button to skip cinematic. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[NewMainMenu]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class URadialSlider> SkipHoldProgress = nullptr;

	/** Contains current time player holds the skip cinematic button. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	float CurrentHoldTime = 0.f;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the widget is removed from the viewport. */
	virtual void NativeDestruct() override;

	/** Called when the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(const struct FGameplayEventData& Payload);

	/** Is called from input while the skip holding button is ongoing. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnCinematicSkipOngoing();

	/** Is called from input on skip cinematic button released (cancelled). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnCinematicSkipReleased();

	/** Is called to skip cinematic on finished holding the skip button or clicked on UI. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnCinematicSkipFinished();
};
