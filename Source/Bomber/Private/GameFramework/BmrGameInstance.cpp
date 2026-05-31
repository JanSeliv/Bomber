// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrGameInstance.h"

// Bomber
#include "DataAssets/BmrGameStateDataAsset.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Subsystems/BmrWidgetsSubsystem.h"

// Steam
#include "AdvancedSteamFriendsLibrary.h"
#include "CreateSessionCallbackProxyAdvanced.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameInstance)

UBmrGameInstance::UBmrGameInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	// Disable auto join (to destroy existing session first), but keep auto travel enabled (to open the level after session is joined)
	bAutoJoinSessionOnAcceptedUserInviteReceived = false;
	bAutoTravelOnAcceptedUserInviteReceived = true;

	// Voice chat is disabled, so don't subscribe to talking-status delegate (avoids voice-interface warning when online voice is unavailable)
	bEnableTalkingStatusDelegate = false;
}

/*********************************************************************************************
 * Overrides and Events
 ********************************************************************************************* */

// Is overridden to listen when first local player is added
int32 UBmrGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	const int32 PlayerIdx = Super::AddLocalPlayer(NewPlayer, UserId);

	if (PlayerIdx == 0)
	{
		checkf(NewPlayer, TEXT("ERROR: [%i] %hs:\n'NewPlayer' is null!"), __LINE__, __FUNCTION__);
		NewPlayer->OnPlayerControllerChanged().AddUObject(this, &ThisClass::OnPlayerControllerReady);
	}

	return PlayerIdx;
}

// Is called on any level when first local player has had a new outer
void UBmrGameInstance::OnPlayerControllerReady_Implementation(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		// Might be null on unpossessing and cleanups, ignore
		return;
	}

	// Unbind from event, as it's designed to be called only for the first time
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	checkf(LocalPlayer, TEXT("ASSERT: [%i] %hs:\n'LocalPlayer' is null!"), __LINE__, __FUNCTION__);
	LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);

	if (UGameplayUtilsLibrary::IsLevelOpened(UBmrGameStateDataAsset::Get().GetStartupLevel()))
	{
		OnStartupLevelReady(PlayerController);
	}
}

// Is called when the startup level is fully loaded and ready
void UBmrGameInstance::OnStartupLevelReady_Implementation(APlayerController* PlayerController)
{
	// Automatically create session on Startup level
	if (UAdvancedSteamFriendsLibrary::IsSteamAvailable())
	{
		TryCreateSession(PlayerController);
	}
	else
	{
		// Launched startup level in offline mode, open main level directly skipping the create session
		UGameplayUtilsLibrary::OpenListenServerLevel(UBmrGameStateDataAsset::Get().GetMainLevel());
	}
}

/*********************************************************************************************
 * Create online session (server)
 ********************************************************************************************* */

// Attempts to create a session, is also called automatically on game startup on empty startup level
void UBmrGameInstance::TryCreateSession(APlayerController* PlayerController)
{
	if (!ensureMsgf(UAdvancedSteamFriendsLibrary::IsSteamAvailable(), TEXT("ASSERT: [%i] %hs:\nAttempted to create session while Steam is not available!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(PlayerController, TEXT("ASSERT: [%i] %hs:\n'PlayerController' is null, failed to create session!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineSessionPtr SessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
	if (!ensureMsgf(SessionInterface, TEXT("ASSERT: [%i] %hs:\n'SessionInterface' is not valid!"), __LINE__, __FUNCTION__)
	    || SessionInterface->GetNamedSession(NAME_GameSession))
	{
		// Session is already created
		return;
	}

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));

	UCreateSessionCallbackProxyAdvanced::CreateAdvancedDefaultSession(this, PlayerController).Activate();
}

// Called when server session is created successfully, e.g: when main level is opened
void UBmrGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Created session!"));

	UGameplayUtilsLibrary::OpenListenServerLevel(UBmrGameStateDataAsset::Get().GetMainLevel());
}

/*********************************************************************************************
 * Join online session (clients)
 ********************************************************************************************* */

// Attempts to join a session
void UBmrGameInstance::TryJoinSession(const FBlueprintSessionResult& SessionToJoin)
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensureMsgf(PlayerController, TEXT("ASSERT: [%i] %hs:\n'PlayerController' is null, failed to create session!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineSessionPtr SessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
	if (!ensureMsgf(SessionInterface, TEXT("ASSERT: [%i] %hs:\n'SessionInterface' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Leave previous session if any
	const FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete, SessionToJoin.OnlineResult);
	SessionInterface->DestroySession(NAME_GameSession, OnDestroySessionCompleteDelegate);

	// Unpossess the camera and hide widgets, so the player can see the loading screen
	PlayerController->SetViewTargetWithBlend(nullptr);

	if (UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem(PlayerController))
	{
		WidgetsSubsystem->SetAllWidgetsVisibility(false);
	}
}

// Session callback when a user accepts an invitation
void UBmrGameInstance::OnSessionInviteAcceptedMaster(const bool bWasSuccessful, int32 LocalPlayer, TSharedPtr<const FUniqueNetId> PersonInviting, const FOnlineSessionSearchResult& SessionToJoin)
{
	Super::OnSessionInviteAcceptedMaster(bWasSuccessful, LocalPlayer, PersonInviting, SessionToJoin);

	if (!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to accept session invite!"));
		return;
	}

	TryJoinSession({SessionToJoin});
}

// Session callback when a user receives an invitation
void UBmrGameInstance::OnSessionInviteReceivedMaster(const FUniqueNetId& PersonInvited, const FUniqueNetId& PersonInviting, const FString& AppId, const FOnlineSessionSearchResult& SessionToJoin)
{
	Super::OnSessionInviteReceivedMaster(PersonInvited, PersonInviting, AppId, SessionToJoin);

	if (!SessionToJoin.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Session invite is invalid!"));
		return;
	}

	TryJoinSession({SessionToJoin});
}

// Called when previous session is destroyed, now can join the new one
void UBmrGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful, FOnlineSessionSearchResult SessionToJoin) const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineSessionPtr SessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
	if (!ensureMsgf(SessionInterface, TEXT("ASSERT: [%i] %hs:\n'SessionInterface' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Joining session!"));

	constexpr int32 LocalUserNum = 0;
	SessionToJoin.Session.SessionSettings.bUsesPresence = true;
	SessionToJoin.Session.SessionSettings.bUseLobbiesIfAvailable = true;
	SessionInterface->JoinSession(LocalUserNum, NAME_GameSession, SessionToJoin);
}