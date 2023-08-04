// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "NewMainMenuSubsystem.generated.h"

/**
 * Provides access to the common runtime data about new Main Menu like current cinematic spots.
 */
UCLASS(BlueprintType, Blueprintable)
class NEWMAINMENU_API UNewMainMenuSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNewMainMenuSubsystem& Get();

	/** Add new Main-Menu spot, so it can be obtained by other objects. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddNewMainMenuSpot(class UNewMainMenuSpotComponent* NewMainMenuSpotComponent);

	/** Returns currently selected Main-Menu spot. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UNewMainMenuSpotComponent* GetActiveMainMenuSpotComponent() const;

protected:
	/** All Main Menu spots with characters placed on the level. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main-Menu Spots"))
	TArray<TObjectPtr<class UNewMainMenuSpotComponent>> MainMenuSpotsInternal;
};
