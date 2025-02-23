// Copyright (c) Yevhenii Selivanov

#include "NMMCheatExtension.h"
//---
#include "NMMUtils.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Components/NMMPlayerControllerComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Data/NMMSaveGameData.h"
#include "GameFramework/MyCheatManager.h"
#include "MyUtilsLibraries/SaveUtilsLibrary.h"
#include "Subsystems/NMMSpotsSubsystem.h"
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

// Makes skins unavailable or allows to apply by index
void UNMMCheatExtension::SetPlayerSkinsAvailable(const FString& SkinAvailabilityMask)
{
	UNMMSpotComponent* CurrentSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
	if (!ensureMsgf(CurrentSpot, TEXT("ASSERT: [%i] %hs:\n'CurrentSpot' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	constexpr int32 MaxSkinBits = 32;
	UMySkeletalMeshComponent& MeshComponent = CurrentSpot->GetMeshChecked();
	const int32 Bitmask = UMyCheatManager::GetBitmaskFromReverseString(SkinAvailabilityMask);
	for (int32 SkinIdx = 0; SkinIdx < MaxSkinBits; ++SkinIdx)
	{
		const bool bIsAvailable = (Bitmask & (1 << SkinIdx)) != 0;
		MeshComponent.SetSkinAvailable(bIsAvailable, SkinIdx);
	}

	CurrentSpot->ApplyMeshOnPlayer();
}
