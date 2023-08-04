// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "NewMainMenuUtils.generated.h"

/**
 * Static helper functions about New Main Menu.
 */
UCLASS()
class NEWMAINMENU_API UNewMainMenuUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the HUD component of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UNewMainMenuHUDComponent* GetHUDComponent();

	/** Returns the widget of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UNewMainMenuWidget* GetMainMenuWidget();
};
