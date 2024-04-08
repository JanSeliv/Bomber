// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "MVVM_MyCharacterBase.generated.h"

/**
 * Contains UI character-related data to be used only by widgets, it can represent player as well as bot.
 * There is own child View Model for each character: UMVVM_MyCharacter0, UMVVM_MyCharacter1, UMVVM_MyCharacter2, UMVVM_MyCharacter3.
 * It does not use single View Model with Character ID as property, but View Model created per each character for next reasons:
 * - It is much easier for the UI designer to work with separate View Models such selecting the right View Model for the right character instead of using Conversion Functions.
 * - It is not the problem since the number of character is always limited.
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
	 * PowerUps
	 ********************************************************************************************* */
public:
	/** Setter and Getter about current amount of speed power-ups. */
	void SetPowerUpSkateN(const FText& NewPowerUpSkate) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpSkateN, NewPowerUpSkate); }
	const FText& GetPowerUpSkateN() const { return PowerUpSkateN; }

	/** Setter and Getter about current amount of max bomb power-ups. */
	void SetPowerUpBombN(const FText& NewPowerUpBomb) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpBombN, NewPowerUpBomb); }
	const FText& GetPowerUpBombN() const { return PowerUpBombN; }

	/** Setter and Getter about current amount of blast radius power-ups. */
	void SetPowerUpFireN(const FText& NewPowerUpFire) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpFireN, NewPowerUpFire); }
	const FText& GetPowerUpFireN() const { return PowerUpFireN; }

	/** Setter and Getter about percentage of the current amount of speed power-ups. */
	void SetPowerUpSkatePercent(float NewPowerUpSkatePercent) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpSkatePercent, NewPowerUpSkatePercent); }
	float GetPowerUpSkatePercent() const { return PowerUpSkatePercent; }

	/** Setter and Getter about percentage of the current amount of max bomb power-ups. */
	void SetPowerUpBombPercent(float NewPowerUpBombPercent) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpBombPercent, NewPowerUpBombPercent); }
	float GetPowerUpBombPercent() const { return PowerUpBombPercent; }

	/** Setter and Getter about percentage of the current amount of blast radius power-ups. */
	void SetPowerUpFirePercent(float NewPowerUpFirePercent) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpFirePercent, NewPowerUpFirePercent); }
	float GetPowerUpFirePercent() const { return PowerUpFirePercent; }

protected:
	/** Current amount of speed power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpSkateN = FText::GetEmpty();

	/** Current amount of max bomb power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpBombN = FText::GetEmpty();

	/** Current amount of blast radius power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpFireN = FText::GetEmpty();

	/** Percentage of the current speed power-up. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	float PowerUpSkatePercent = 0.f;

	/** Percentage of the current amount of max bomb power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	float PowerUpBombPercent = 0.f;

	/** Percentage of the current amount of blast radius power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	float PowerUpFirePercent = 0.f;

	/** Called when power-ups were updated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPowerUpsChanged(const struct FPowerUp& NewPowerUps);

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/**  Is called when this View Model is constructed.
	* Is used for bindings to the changes in other systems in order to update own data. */
	virtual void OnViewModelConstruct_Implementation(const UUserWidget* UserWidget) override;

	/** Is called when this View Model is destructed. */
	virtual void OnViewModelDestruct_Implementation() override;

	/** Called when local player character was possessed, so we can bind to data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCharacterWithIDPossessed(class APlayerCharacter* PlayerCharacter, int32 CharacterID);
};

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
