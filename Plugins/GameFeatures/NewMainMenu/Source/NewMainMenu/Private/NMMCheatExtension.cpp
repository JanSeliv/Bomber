// Copyright (c) Yevhenii Selivanov

#include "NMMCheatExtension.h"
//---
#include "NMMUtils.h"
#include "Components/NMMPlayerControllerComponent.h"
#include "Data/NMMSaveGameData.h"
#include "MyUtilsLibraries/SaveUtilsLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCheatExtension)

// Removes a save file of the New Main Menu
void UNMMCheatExtension::ResetNewMainMenuSaves()
{
	UNMMPlayerControllerComponent* MenuControllerComp = UNMMUtils::GetPlayerControllerComponent();
	UNMMSaveGameData* CurrentSave = MenuControllerComp ? MenuControllerComp->GetSaveGameData() : nullptr;
	if (CurrentSave)
	{
		USaveGame* NewSave = USaveUtilsLibrary::ResetSaveGameData(CurrentSave, UNMMSaveGameData::GetSaveSlotName(), UNMMSaveGameData::GetSaveSlotIndex());
		MenuControllerComp->SetSaveGameData(NewSave);
	}
}
