// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "PlayerName3DWidget.generated.h"

/**
 * Is 3D widget above the player character with its name.
 */
UCLASS(Abstract)
class BOMBER_API UPlayerName3DWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Is overridden to hide dependent 3D widget components along with this widget. */
	virtual void SetVisibility(ESlateVisibility InVisibility) override;

	/** Is overridden to perform cleanup. */
	virtual void BeginDestroy() override;

	/*********************************************************************************************
	 * Player Name
	 ********************************************************************************************* */
public:
	/** Returns the player name from the widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FName GetPlayerName() const;

	/** Sets player name to the widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetPlayerName(FName NewPlayerName);

protected:
	/** The text block with player name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameTextWidget = nullptr;

	/*********************************************************************************************
	 * Player Owner
	 ********************************************************************************************* */
public:
	/** Returns the player character owner of the widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class APlayerCharacter* GetPlayerOwner() const { return PlayerOwnerInternal; }

	/** Sets the player character to the widget.
	 * It's essential to set since it's used by AI characters as well, so it's only a way to get correct owner. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetPlayerOwner(class APlayerCharacter* NewPlayer);

protected:
	/** Contains cached player character owner, which can be AI or Player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "NAME"))
	TObjectPtr<class APlayerCharacter> PlayerOwnerInternal = nullptr;
};
