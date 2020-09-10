// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "GameFramework/HUD.h"
#include "Bomber.h"
//---
#include "MyHUD.generated.h"

/**
 * The custom HUD class. Also manages other widgets.
 */
UCLASS()
class BOMBER_API AMyHUD final : public AHUD
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public properties
	* --------------------------------------------------- */

	/** The UMG class of the Level.
	 * @todo Replace to the UI Data Asset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TSubclassOf<class UUserWidget> InGameWidgetClass;  // [B]

	/** The current widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Ñ++")
	class UUserWidget* InGameWidget;  //[G]

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/* Sets default values for this HUD's properties. */
	AMyHUD();

protected:
	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when the game starts. Created widget. */
	virtual void BeginPlay() override;

	/** */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
    void OnGameStarted(ECurrentGameState CurrentGameState);
};
