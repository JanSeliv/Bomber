// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrGameInstance.h"

// Bomber
#include "DataAssets/BmrGameStateDataAsset.h"
#include "Subsystems/BmrWidgetsSubsystem.h"

// Steam
#include "AdvancedSteamFriendsLibrary.h"
#include "CreateSessionCallbackProxyAdvanced.h"

// UE
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameInstance)

/*********************************************************************************************
 * Overrides and Events
 ********************************************************************************************* */

UBmrGameInstance::UBmrGameInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	bAutoJoinSessionOnAcceptedUserInviteReceived = false;
}

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

// Is called when first local player has had a new outer
void UBmrGameInstance::OnPlayerControllerReady(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		// Might be null on unpossessing and cleanups, ignore
		return;
	}

	TryCreateSession(PlayerController);

	// Unbind from event, as it's designed to be called only for the first time
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	checkf(LocalPlayer, TEXT("ASSERT: [%i] %hs:\n'LocalPlayer' is null!"), __LINE__, __FUNCTION__);
	LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);
}

/*********************************************************************************************
 * Online Sessions
 ********************************************************************************************* */

// Attempts to create a session
void UBmrGameInstance::TryCreateSession(APlayerController* PlayerController)
{
	if (!UAdvancedSteamFriendsLibrary::IsSteamAvailable()
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

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Created session!"));

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));

	UCreateSessionCallbackProxyAdvanced::CreateAdvancedDefaultSession(this, PlayerController).Activate();
}

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

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Joining session!"));

	// Listen when session is destroyed to join the session
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	OnDestroySessionCompleteDelegate.BindWeakLambda(this, [WeakSession = SessionInterface.ToWeakPtr(), SessionToJoin](FName SessionName, bool bWasSuccessful)
	{
		// Session is destroyed, join session
		if (const TSharedPtr<IOnlineSession> SessionInterface = WeakSession.Pin())
		{
			FOnlineSessionSearchResult ModResult = SessionToJoin.OnlineResult;
			ModResult.Session.SessionSettings.bUsesPresence = true;
			ModResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
			SessionInterface->JoinSession(0, NAME_GameSession, ModResult);
		}
	});
	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

	// Leave previous session if any
	SessionInterface->DestroySession(NAME_GameSession);

	// Unpossess the camera and hide widgets, so the player can see the loading screen
	PlayerController->SetViewTargetWithBlend(nullptr);

	if (UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem(PlayerController))
	{
		WidgetsSubsystem->SetAllWidgetsVisibility(false);
	}
}

// Called when server session is created successfully, e.g: when main level is opened
void UBmrGameInstance::OnCreateSessionComplete(FName Name, bool bArg) const
{
	const UBmrGameStateDataAsset& GameStateDataAsset = UBmrGameStateDataAsset::Get();
	const FString CurrentLevelName = GetNameSafe(GetWorld());
	if (CurrentLevelName.Contains(GameStateDataAsset.GetStartupLevel().GetAssetName()))
	{
		static const FString Options = TEXT("Listen");
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameStateDataAsset.GetMainLevel(), /*bAbsolute=*/true, Options);
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