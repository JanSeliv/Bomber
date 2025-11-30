// Copyright (c) Yevhenii Selivanov

#pragma once

#include "AdvancedFriendsGameInstance.h"

#include "BmrGameInstance.generated.h"

/**
 * The game instance class that is primarily used to manage the online sessions (create, join, destroy etc).
 */
UCLASS()
class BOMBER_API UBmrGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

	UBmrGameInstance(const FObjectInitializer& ObjectInitializer);

	/*********************************************************************************************
	 * Overrides and Events
	 ********************************************************************************************* */
protected:
	/** Is overridden to listen when first local player is added. */
	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;

	/** Is called when first local player has had a new outer. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPlayerControllerReady(class APlayerController* PlayerController);

	/*********************************************************************************************
	 * Online Sessions
	 * Flow: TryCreateSession → Open?listen → MainLevel
	 ********************************************************************************************* */
public:
	/** Attempts to create a session. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void TryCreateSession(class APlayerController* PlayerController);

	/** Attempts to join a session. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void TryJoinSession(const struct FBlueprintSessionResult& SessionToJoin);

protected:
	/** Called when server session is created successfully, e.g: when main level is opened. */
	void OnCreateSessionComplete(FName Name, bool bArg) const;

	/** Session callback when a user accepts an invitation. */
	virtual void OnSessionInviteAcceptedMaster(const bool bWasSuccessful, int32 LocalPlayer, TSharedPtr<const FUniqueNetId> PersonInviting, const FOnlineSessionSearchResult& SessionToJoin) override;

	/** Session callback when a user receives an invitation. */
	virtual void OnSessionInviteReceivedMaster(const FUniqueNetId& PersonInvited, const FUniqueNetId& PersonInviting, const FString& AppId, const FOnlineSessionSearchResult& SessionToJoin) override;
};