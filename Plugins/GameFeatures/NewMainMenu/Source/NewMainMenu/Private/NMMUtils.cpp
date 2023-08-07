// Copyright (c) Yevhenii Selivanov

#include "NewMainMenuUtils.h"
//---
#include "UI/MyHUD.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "Components/NewMainMenuHUDComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuUtils)

// Returns the HUD component of the Main Menu
UNewMainMenuHUDComponent* UNewMainMenuUtils::GetHUDComponent()
{
	const AMyHUD* MyHUD = UMyBlueprintFunctionLibrary::GetMyHUD();
	return MyHUD ? MyHUD->FindComponentByClass<UNewMainMenuHUDComponent>() : nullptr;
}

// Returns the widget of the Main Menu.
UNewMainMenuWidget* UNewMainMenuUtils::GetMainMenuWidget()
{
	const UNewMainMenuHUDComponent* HUDComponent = GetHUDComponent();
	return HUDComponent ? HUDComponent->GetMainMenuWidget() : nullptr;
}
