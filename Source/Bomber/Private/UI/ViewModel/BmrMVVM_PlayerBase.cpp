// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/BmrMVVM_PlayerBase.h"

// Bomber
#include "AdvancedSteamFriendsLibrary.h"
#include "DataAssets/BmrUIDataAsset.h"
#include "GameFramework/BmrPlayerState.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMVVM_PlayerBase)

// Is overridden to prevent constructing this View Model, but only child classes
bool UBmrMVVM_PlayerBase::CanConstructViewModel_Implementation() const
{
	return Super::CanConstructViewModel_Implementation()
	       && GetPlayerId() != INDEX_NONE;
}

/*********************************************************************************************
 * Nickname
 ********************************************************************************************* */

// Called when changed Character's name
void UBmrMVVM_PlayerBase::OnNicknameChanged_Implementation(FName NewNickname)
{
	SetNickname(FText::FromName(NewNickname));
}

/*********************************************************************************************
 * Is Character Dead
 ********************************************************************************************* */

// Called when changed character Dead status is changed
void UBmrMVVM_PlayerBase::OnPlayerDeadChanged_Implementation(bool bIsDead)
{
	SetIsDeadVisibility(bIsDead ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

/*********************************************************************************************
 * Avatar (Human / Bot / Online)
 ********************************************************************************************* */

// Assigns current avatar based on player type
void UBmrMVVM_PlayerBase::UpdateAvatar()
{
	const ABmrPlayerState* MyPlayerState = UBmrBlueprintFunctionLibrary::GetPlayerState(GetPlayerId());
	const EBmrPlayerType PlayerType = MyPlayerState ? MyPlayerState->GetPlayerType() : EBmrPlayerType::None;
	UTexture2D* NewAvatar = UBmrUIDataAsset::Get().GetDefaultAvatar(PlayerType);
	if (!ensureMsgf(NewAvatar, TEXT("ASSERT: [%i] %hs:\n'NewAvatar' is null, can not obtain any!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (PlayerType == EBmrPlayerType::Bot)
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
void UBmrMVVM_PlayerBase::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	BIND_ON_PLAYER_STATE_READY_ID(this, ThisClass::OnPlayerStateReady, GetPlayerId());
}

// Is called when this View Model is destructed
void UBmrMVVM_PlayerBase::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (ABmrPlayerState* PlayerState = UBmrBlueprintFunctionLibrary::GetPlayerState(GetPlayerId()))
	{
		PlayerState->OnPlayerNameChanged.RemoveAll(this);
		PlayerState->OnPlayerDeadChanged.RemoveAll(this);
	}
}

// Called when any player state is initialized and its assigned character is ready
void UBmrMVVM_PlayerBase::OnPlayerStateReady_Implementation(ABmrPlayerState* PlayerState, int32 PlayerId)
{
	checkf(PlayerId == GetPlayerId(), TEXT("ERROR: [%i] %hs:\n'PlayerId' is different than owned!"), __LINE__, __FUNCTION__);

	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);

	PlayerState->OnPlayerNameChanged.AddUniqueDynamic(this, &ThisClass::OnNicknameChanged);
	OnNicknameChanged(*PlayerState->GetPlayerName());

	PlayerState->OnPlayerDeadChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerDeadChanged);
	OnPlayerDeadChanged(PlayerState->IsPlayerDead());

	PlayerState->OnPlayerTypeChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerTypeChanged);
	OnPlayerTypeChanged(PlayerState->GetPlayerType());
}

// Called when changed character Bot status is changed, applies both bot and human visibility
void UBmrMVVM_PlayerBase::OnPlayerTypeChanged_Implementation(EBmrPlayerType PlayerType)
{
	UpdateAvatar();
}