// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyCharacterBase.h"
//---
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_MyCharacterBase)

// Is overridden to prevent constructing this View Model, but only child classes
bool UMVVM_MyCharacterBase::CanConstructViewModel_Implementation() const
{
	return Super::CanConstructViewModel_Implementation()
		   && GetCharacterId() != INDEX_NONE;
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

// Called when own player character was spawned and possessed, so we can bind to data
void UMVVM_MyCharacterBase::OnCharacterReady_Implementation(APlayerCharacter* PlayerCharacter, int32 CharacterID)
{
	if (CharacterID != GetCharacterId())
	{
		// This View Model is not for this character
		return;
	}

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