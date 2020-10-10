// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "GameFramework/HUD.h"
//---
#include "MyHUD.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UUIDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public
	 * --------------------------------------------------- */

	/** Default constructor. */
	UUIDataAsset() = default;

	/** Get UUIDataAsset::InGameWidgetInternal.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE TSubclassOf<class UUserWidget> GetInGameClass() const { return InGameClassInternal; }

protected:
	/** The class of a In-Game Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "In-Game Widget", ShowOnlyInnerProperties))
	TSubclassOf<class UUserWidget> InGameClassInternal; //[D]

};


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

	/** The current widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Ñ++")
	class UInGameWidget* InGameWidget;  //[G]

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/* Sets default values for this HUD's properties. */
	AMyHUD() = default;

protected:
	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when the game starts. Created widget. */
	virtual void BeginPlay() override;
};
