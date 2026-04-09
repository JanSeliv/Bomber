// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"

// NMM
#include "Data/NmmStateTag.h"

#include "NmmMVVM_MenuViewModel.generated.h"

/**
 * Contains New Main Menu data to be used only by menu widgets.
 */
UCLASS(DisplayName = "NMM Menu View Model")
class NEWMAINMENU_API UNmmMVVM_MenuViewModel : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Current Menu State
	 ********************************************************************************************* */
public:
	/** Setter and Getter about the current menu state tag. */
	UFUNCTION()
	void SetCurrentMenuStateTag(FNmmStateTag NewCurrentMenuStateTag) { UE_MVVM_SET_PROPERTY_VALUE(CurrentMenuStateTag, NewCurrentMenuStateTag); }

	FNmmStateTag GetCurrentMenuStateTag() const { return CurrentMenuStateTag; }

protected:
	/** Represents the current menu state.
	 * Is commonly used by 'UNMMUtils::GetVisibilityByMenuStateTag' to show or hide own widget. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[NewMainMenu]")
	FNmmStateTag CurrentMenuStateTag = FNmmStateTag::None;

	/** Called when the Main Menu state was changed, updates own tag. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	void OnMenuStateChanged(const struct FGameplayEventData& Payload);

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called when this View Model is constructed.
	 * Is used for bindings to the changes in other systems in order to update own data. */
	virtual void OnViewModelConstruct_Implementation(const UUserWidget* UserWidget) override;

	/** Is called when this View Model is destructed. */
	virtual void OnViewModelDestruct_Implementation() override;
};
