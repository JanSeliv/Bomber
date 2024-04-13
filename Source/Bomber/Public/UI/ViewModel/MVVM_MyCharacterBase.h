// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "Components/SlateWrapperTypes.h"
//---
#include "MVVM_MyCharacterBase.generated.h"

/**
 * Contains UI character-related data to be used only by widgets, it can represent player as well as bot.
 */
UCLASS(Abstract, DisplayName = "[Abstract] My Character Base View Model")
class BOMBER_API UMVVM_MyCharacterBase : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

public:
	/** Has to be overridden in child classes to provide the character ID. */
	virtual FORCEINLINE int32 GetCharacterId() const PURE_VIRTUAL(UMVVM_BaseCharacter::GetCharacterId, return INDEX_NONE;);

	/** Is overridden to prevent constructing this View Model, but only child classes. */
	virtual bool CanConstructViewModel_Implementation() const override;

	/*********************************************************************************************
	 * Nickname
	 ********************************************************************************************* */
public:
	/** Setter and Getter about Character's name. */
	void SetNickname(const FText& NewNickname) { UE_MVVM_SET_PROPERTY_VALUE(Nickname, NewNickname); }
	const FText& GetNickname() const { return Nickname; }

protected:
	/** Character's name. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText Nickname = FText::GetEmpty();

	/** Called when changed Character's name. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnNicknameChanged(FName NewNickname);

	/*********************************************************************************************
	 * Is Character Dead
	 ********************************************************************************************* */
public:
	/** Setter and Getter character dead visibility, should be 'Visible' when character is dead, 'Collapsed' otherwise. */
	void SetIsDeadVisibility(ESlateVisibility NewIsDeadVisibility) { UE_MVVM_SET_PROPERTY_VALUE(IsDeadVisibility, NewIsDeadVisibility); }
	ESlateVisibility GetIsDeadVisibility() const { return IsDeadVisibility; }

protected:
	/** Is 'Visible' when character is dead, collapsed otherwise. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	ESlateVisibility IsDeadVisibility = ESlateVisibility::Collapsed;

	/** Called when changed character Dead status is changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCharacterDeadChanged(bool bIsCharacterDead);

	/*********************************************************************************************
	 * Is Human / Bot
	 ********************************************************************************************* */
public:
	/** Setter and Getter is character human controlled, should be 'Visible' when character is human. */
	void SetIsHumanVisibility(ESlateVisibility NewIsHumanVisibility) { UE_MVVM_SET_PROPERTY_VALUE(IsHumanVisibility, NewIsHumanVisibility); }
	ESlateVisibility GetIsHumanVisibility() const { return IsHumanVisibility; }

	/** Setter and Getter character bot controlled, should be 'Visible' when character is bot. */
	void SetIsBotVisibility(ESlateVisibility NewIsBotVisibility) { UE_MVVM_SET_PROPERTY_VALUE(IsBotVisibility, NewIsBotVisibility); }
	ESlateVisibility GetIsBotVisibility() const { return IsBotVisibility; }

protected:
	/** Is 'Visible' when character is human. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	ESlateVisibility IsHumanVisibility = ESlateVisibility::Collapsed;

	/** Is 'Visible' when character is bot. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	ESlateVisibility IsBotVisibility = ESlateVisibility::Collapsed;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/**  Is called when this View Model is constructed.
	* Is used for bindings to the changes in other systems in order to update own data. */
	virtual void OnViewModelConstruct_Implementation(const UUserWidget* UserWidget) override;

	/** Is called when this View Model is destructed. */
	virtual void OnViewModelDestruct_Implementation() override;

	/** Called when own player character was possessed, so we can bind to data. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCharacterReady(class APlayerCharacter* PlayerCharacter, int32 CharacterID);
};

/*********************************************************************************************
 * Below View Models per each character: UMVVM_MyCharacter0, UMVVM_MyCharacter1, UMVVM_MyCharacter2, UMVVM_MyCharacter3.
 * It's done in such 'hardcoded' way for next reasons:
 * - It is much easier for the UI designer to work with separate View Models 
 * by selecting the right View Model for the right character instead of struggling with Conversion Functions.
 * - It is not the problem since the number of character is always limited.
 ********************************************************************************************* */

UCLASS(DisplayName = "My Character #0 View Model")
class BOMBER_API UMVVM_MyCharacter0 : public UMVVM_MyCharacterBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetCharacterId() const override { return 0; }
};

UCLASS(DisplayName = "My Character #1 View Model")
class BOMBER_API UMVVM_MyCharacter1 : public UMVVM_MyCharacterBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetCharacterId() const override { return 1; }
};

UCLASS(DisplayName = "My Character #2 View Model")
class BOMBER_API UMVVM_MyCharacter2 : public UMVVM_MyCharacterBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetCharacterId() const override { return 2; }
};

UCLASS(DisplayName = "My Character #3 View Model")
class BOMBER_API UMVVM_MyCharacter3 : public UMVVM_MyCharacterBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetCharacterId() const override { return 3; }
};