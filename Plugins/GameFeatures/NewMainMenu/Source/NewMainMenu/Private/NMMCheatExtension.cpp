// Copyright (c) Yevhenii Selivanov

#include "NMMCheatExtension.h"
//---
#include "NMMUtils.h"
#include "Components/NMMPlayerControllerComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCheatExtension)

// Removes a save file of the New Main Menu
void UNMMCheatExtension::ResetNewMainMenuSaves()
{
	if (UNMMPlayerControllerComponent* MenuControllerComp = UNMMUtils::GetPlayerControllerComponent())
	{
		MenuControllerComp->ResetSaveGameData();
	}
}
