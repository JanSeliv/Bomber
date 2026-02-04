// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"

// UE
#include "Components/SlateWrapperTypes.h"

#include "BmrMVVM_PlayerBase.generated.h"

class UTexture2D;

enum class EBmrPlayerType : uint8;

/**
 * Contains UI character-related data to be used only by widgets, it can represent player as well as bot.
 */
UCLASS(Abstract, DisplayName = "[Abstract] Bomber Player Base View Model")
class BOMBER_API UBmrMVVM_PlayerBase : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

public:
	/** Has to be overridden in child classes to provide the character ID. */
	virtual FORCEINLINE int32 GetPlayerId() const PURE_VIRTUAL(ThisClass::GetPlayerId, return INDEX_NONE;);

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
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	FText Nickname = FText::GetEmpty();

	/** Called when changed Character's name. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
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
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	ESlateVisibility IsDeadVisibility = ESlateVisibility::Collapsed;

	/** Called when changed character Dead status is changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPlayerDeadChanged(bool bIsDead);

	/*********************************************************************************************
	 * Avatar (Human / Bot / Online)
	 ********************************************************************************************* */
public:
	/** Setter and Getter about character's avatar, is always valid: default human, bot, or player's online avatar. */
	void SetAvatar(UTexture2D* NewAvatar) { UE_MVVM_SET_PROPERTY_VALUE(Avatar, NewAvatar); }
	UTexture2D* GetAvatar() const { return Avatar; }

	/** Assigns current avatar based on player type. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void UpdateAvatar();

protected:
	/** Character's avatar, is always valid: default human, bot, or player's online avatar. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	TObjectPtr<UTexture2D> Avatar = nullptr;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/**  Is called when this View Model is constructed.
	 * Is used for bindings to the changes in other systems in order to update own data. */
	virtual void OnViewModelConstruct_Implementation(const UUserWidget* UserWidget) override;

	/** Is called when this View Model is destructed. */
	virtual void OnViewModelDestruct_Implementation() override;

	/** Called when the pawn is initialized and its assigned character is ready */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPawnReady(const struct FGameplayEventData& Payload);

	/** Called when changed character Bot status is changed, applies both bot and human visibility. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPlayerTypeChanged(EBmrPlayerType PlayerType);
};

/*********************************************************************************************
 * Below View Models per each character: UBmrMVVM_Player0, UBmrMVVM_Player1, UBmrMVVM_Player2, UBmrMVVM_Player3.
 * It's done in such 'hardcoded' way for next reasons:
 * - It is much easier for the UI designer to work with separate View Models
 * by selecting the right View Model for the right character instead of struggling with Conversion Functions.
 * - It is not the problem since the number of character is always limited.
 ********************************************************************************************* */

UCLASS(DisplayName = "Bomber Player #0 View Model")
class BOMBER_API UBmrMVVM_Player0 : public UBmrMVVM_PlayerBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetPlayerId() const override { return 0; }
};

UCLASS(DisplayName = "Bomber Player #1 View Model")
class BOMBER_API UBmrMVVM_Player1 : public UBmrMVVM_PlayerBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetPlayerId() const override { return 1; }
};

UCLASS(DisplayName = "Bomber Player #2 View Model")
class BOMBER_API UBmrMVVM_Player2 : public UBmrMVVM_PlayerBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetPlayerId() const override { return 2; }
};

UCLASS(DisplayName = "Bomber Player #3 View Model")
class BOMBER_API UBmrMVVM_Player3 : public UBmrMVVM_PlayerBase
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE int32 GetPlayerId() const override { return 3; }
};