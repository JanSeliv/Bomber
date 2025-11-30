// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyCharacterBase.h"

// Bomber
#include "AdvancedSteamFriendsLibrary.h"
#include "DataAssets/UIDataAsset.h"
#include "GameFramework/MyPlayerState.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

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
 * Avatar (Human / Bot / Online)
 ********************************************************************************************* */

// Assigns current avatar based on player type
void UMVVM_MyCharacterBase::UpdateAvatar()
{
	const AMyPlayerState* MyPlayerState = UMyBlueprintFunctionLibrary::GetMyPlayerState(GetCharacterId());
	const EPlayerType PlayerType = MyPlayerState ? MyPlayerState->GetPlayerType() : EPlayerType::None;
	UTexture2D* NewAvatar = UUIDataAsset::Get().GetDefaultAvatar(PlayerType);
	if (!ensureMsgf(NewAvatar, TEXT("ASSERT: [%i] %hs:\n'NewAvatar' is null, can not obtain any!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (PlayerType == EPlayerType::Bot)
	{
		// Set default bot avatar
		SetAvatar(NewAvatar);
		return;
	}

	if (UAdvancedSteamFriendsLibrary::IsOverlayEnabled())
	{
		// Try to obtain online avatar, if not found - human default will be used
		EBlueprintAsyncResultSwitch Result = EBlueprintAsyncResultSwitch::OnFailure;
		UTexture2D* OnlineAvatar = UAdvancedSteamFriendsLibrary::GetSteamFriendAvatar(MyPlayerState->GetUniqueId(), /*out*/ Result);
		if (OnlineAvatar
		    && Result == EBlueprintAsyncResultSwitch::OnSuccess)
		{
			// Online avatar is found
			NewAvatar = OnlineAvatar;
		}
	}

	// Set human avatar
	SetAvatar(NewAvatar);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when the view is constructed
void UMVVM_MyCharacterBase::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	BIND_ON_PLAYER_STATE_READY_ID(this, ThisClass::OnPlayerStateReady, GetCharacterId());
}

// Is called when this View Model is destructed
void UMVVM_MyCharacterBase::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (AMyPlayerState* PlayerState = UMyBlueprintFunctionLibrary::GetMyPlayerState(GetCharacterId()))
	{
		PlayerState->OnPlayerNameChanged.RemoveAll(this);
		PlayerState->OnCharacterDeadChanged.RemoveAll(this);
	}
}

// Called when any player state is initialized and its assigned character is ready
void UMVVM_MyCharacterBase::OnPlayerStateReady_Implementation(AMyPlayerState* PlayerState, int32 CharacterID)
{
	checkf(CharacterID == GetCharacterId(), TEXT("ERROR: [%i] %hs:\n'CharacterID' is different than owned!"), __LINE__, __FUNCTION__);

	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);

	PlayerState->OnPlayerNameChanged.AddUniqueDynamic(this, &ThisClass::OnNicknameChanged);
	OnNicknameChanged(*PlayerState->GetPlayerName());

	PlayerState->OnCharacterDeadChanged.AddUniqueDynamic(this, &ThisClass::OnCharacterDeadChanged);
	OnCharacterDeadChanged(PlayerState->IsCharacterDead());

	PlayerState->OnPlayerTypeChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerTypeChanged);
	OnPlayerTypeChanged(PlayerState->GetPlayerType());
}

// Called when changed character Bot status is changed, applies both bot and human visibility
void UMVVM_MyCharacterBase::OnPlayerTypeChanged_Implementation(EPlayerType PlayerType)
{
	UpdateAvatar();
}