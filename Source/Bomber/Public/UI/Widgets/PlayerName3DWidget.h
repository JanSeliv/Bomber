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

public:
	/** Sets player name to the widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetPlayerName(FName NewPlayerName);

protected:
	/** The text block with player name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameTextWidget = nullptr;
};
