// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyCharacterBase.h"
//---
#include "DataAssets/ItemDataAsset.h"
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_MyCharacterBase)

/*********************************************************************************************
 * PowerUps
 ********************************************************************************************* */

// Is overridden to prevent constructing this View Model, but only child classes
bool UMVVM_MyCharacterBase::CanConstructViewModel_Implementation() const
{
	return Super::CanConstructViewModel_Implementation()
	       && GetCharacterId() != INDEX_NONE;
}

// Called when power-ups were updated
void UMVVM_MyCharacterBase::OnPowerUpsChanged_Implementation(const FPowerUp& NewPowerUps)
{
	SetPowerUpSkateN(FText::AsNumber(NewPowerUps.SkateN));
	SetPowerUpBombN(FText::AsNumber(NewPowerUps.BombN));
	SetPowerUpFireN(FText::AsNumber(NewPowerUps.FireN));

	const int32 MaxPowerUps = static_cast<float>(UItemDataAsset::Get().GetMaxAllowedItemsNum());
	checkf(MaxPowerUps > 0.f, TEXT("ERROR: [%i] %s:\n'MaxPowerUps > 0' is null!"), __LINE__, *FString(__FUNCTION__));
	SetPowerUpSkatePercent(static_cast<float>(NewPowerUps.SkateN) / MaxPowerUps);
	SetPowerUpBombPercent(static_cast<float>(NewPowerUps.BombN) / MaxPowerUps);
	SetPowerUpFirePercent(static_cast<float>(NewPowerUps.FireN) / MaxPowerUps);
}

/*********************************************************************************************
 * Nickname
 ********************************************************************************************* */

// Called when changed Character's name
void UMVVM_MyCharacterBase::OnNicknameChanged_Implementation(FName NewNickname)
{
	SetNickname(FText::FromName(NewNickname));
}

/*********************************************************************************************
 * Is Character Dead
 ********************************************************************************************* */

// Called when changed character Dead status is changed
void UMVVM_MyCharacterBase::OnCharacterDeadChanged_Implementation(bool bIsCharacterDead)
{
	SetIsDeadVisibility(bIsCharacterDead ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when the view is constructed
void UMVVM_MyCharacterBase::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	BIND_AND_CALL_ON_CHARACTER_READY(this, ThisClass::OnCharacterReady, GetCharacterId());
}

// Is called when this View Model is destructed
void UMVVM_MyCharacterBase::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (APlayerCharacter* Character = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter())
	{
		Character->OnPowerUpsChanged.RemoveAll(this);

		if (AMyPlayerState* PlayerState = Character->GetPlayerState<AMyPlayerState>())
		{
			PlayerState->OnPlayerNameChanged.RemoveAll(this);
			PlayerState->OnCharacterDeadChanged.RemoveAll(this);
		}
	}
}

// Called when local player character was spawned and possessed, so we can bind to data
void UMVVM_MyCharacterBase::OnCharacterReady(APlayerCharacter* PlayerCharacter, int32 CharacterID)
{
	if (CharacterID != GetCharacterId())
	{
		// This View Model is not for this character
		return;
	}

	checkf(PlayerCharacter, TEXT("ERROR: [%i] %s:\n'PlayerCharacter' is null!"), __LINE__, *FString(__FUNCTION__));
	PlayerCharacter->OnPowerUpsChanged.AddUniqueDynamic(this, &ThisClass::OnPowerUpsChanged);

	AMyPlayerState* PlayerState = PlayerCharacter->GetPlayerState<AMyPlayerState>();
	if (ensureAlwaysMsgf(PlayerState, TEXT("ASSERT: [%i] %s:\n'CharacterState' is null!"), __LINE__, *FString(__FUNCTION__)))
	{
		PlayerState->OnPlayerNameChanged.AddUniqueDynamic(this, &ThisClass::OnNicknameChanged);
		OnNicknameChanged(PlayerState->GetPlayerFNameCustom());

		PlayerState->OnCharacterDeadChanged.AddUniqueDynamic(this, &ThisClass::OnCharacterDeadChanged);
		OnCharacterDeadChanged(PlayerState->IsCharacterDead());

		// Set by default Human\Bot visibility as there is no support for joining during the match
		const bool IsABot = PlayerState->IsABot();
		SetIsHumanVisibility(IsABot ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		SetIsBotVisibility(IsABot ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}