// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

#include "BmrPlayerNameWidget.generated.h"

/**
 * Represents the player nickname, is used by human and AI characters, both in 3D and 2D UI.
 */
UCLASS(Abstract)
class BOMBER_API UBmrPlayerNameWidget : public UUserWidget
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Player Name
	 ********************************************************************************************* */
public:
	/** Returns the player name from the widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FText GetPlayerName() const;

	/** Sets player name to the widget. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetPlayerName(const FText& NewPlayerName);

protected:
	/** The text block with player name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameTextWidget = nullptr;

	/** Background image for nameplate styling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UImage> BackgroundImageWidget = nullptr;

	/*********************************************************************************************
	 * Player ID
	 ********************************************************************************************* */
public:
	/** Returns ID of the player character owner with which this widget is associated. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetAssociatedPlayerId() const { return AssociatedPlayerId; }

	/** Sets the player ID to the widget.
	 * It's essential to set since it's used by AI characters as well, so it's only a way to get correct owner. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetAssociatedPlayerId(int32 NewPlayerId);

	/** Sets the background material for the nameplate */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetBackgroundMaterial(int32 PlayerId);

protected:
	/** ID of the player character owner with which this widget is associated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 AssociatedPlayerId = INDEX_NONE;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
public:
	/** Called by both the game and the editor.  Allows users to run initial setup for their widgets to better preview. */
	virtual void NativePreConstruct() override;
};
