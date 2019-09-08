// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "GameFramework/HUD.h"

#include "MyHUD.generated.h"

/**
 * The custom HUD class. Also manages other widgets.
 */
UCLASS()
class BOMBER_API AMyHUD final : public AHUD
{
	GENERATED_BODY()

public:
	/** The UMG class of the Menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TSubclassOf<class UUserWidget> UmgMenuClass;  // [B]

	/** The UMG class of the Level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TSubclassOf<class UUserWidget> UmgLevelClass;  // [B]

	/** The current widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Ñ++")
	class UUserWidget* CreatedWidget;  //[G]

	/* Sets default values for this HUD's properties. */
	AMyHUD();

protected:
	/** Called when the game starts. Created widget. */
	virtual void BeginPlay() override;
};
