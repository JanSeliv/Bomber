// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyCheatManager.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Components/MyCameraComponent.h"
#include "Controllers/MyAIController.h"
#include "Controllers/MyDebugCameraController.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameModeBase.h"
#include "LevelActors/BoxActor.h"
#include "LevelActors/PlayerCharacter.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/LevelActorsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyCheatManager)

// Default constructor
UMyCheatManager::UMyCheatManager()
{
	DebugCameraControllerClass = AMyDebugCameraController::StaticClass();
}

/*********************************************************************************************
 * Utils
 ********************************************************************************************* */

// Returns bitmask from reverse bitmask in string
int32 UMyCheatManager::GetBitmaskFromReverseString(const FString& ReverseBitmaskStr)
{
	int32 Bitmask = 0;
	int32 Index = 0;
	for (int32 It = 0; It < ReverseBitmaskStr.Len(); ++It)
	{
		FString Char = ReverseBitmaskStr.Mid(It, 1);
		if (Char.IsNumeric())
		{
			const int32 Bit = !FCString::Atoi(*Char) ? 0 : 1;
			Bitmask |= Bit << Index++;
		}
	}

	return Bitmask;
}

// Returns bitmask by actor types in string
int32 UMyCheatManager::GetBitmaskFromActorTypesString(const FString& ActorTypesBitmaskStr)
{
	if (ActorTypesBitmaskStr.IsEmpty())
	{
		return 0;
	}

	static const FString Delimiter = TEXT(" ");
	TArray<FString> ActorTypesStrings;
	ActorTypesBitmaskStr.ParseIntoArray(ActorTypesStrings, *Delimiter);

	const static FString ActorTypeEnumPathName = TEXT("/Script/Bomber.EActorType");
	static const UEnum* ActorTypeEnumClass = UClass::TryFindTypeSlow<UEnum>(ActorTypeEnumPathName, EFindFirstObjectOptions::ExactClass);
	if (!ensureMsgf(ActorTypeEnumClass, TEXT("%s: 'ActorTypeEnumClass' is not found by next path: %s"), *FString(__FUNCTION__), *ActorTypeEnumPathName))
	{
		return 0;
	}

	int32 Bitmask = 0;
	for (const FString& ActorTypeStrIt : ActorTypesStrings)
	{
		const int32 EnumFlag = ActorTypeEnumClass->GetValueByNameString(ActorTypeStrIt);
		if (EnumFlag != INDEX_NONE)
		{
			Bitmask |= EnumFlag;
		}
	}

	return Bitmask;
}

/*********************************************************************************************
 * Destroy
 ********************************************************************************************* */

// Destroy all specified level actors on the map
void UMyCheatManager::DestroyAllByType(EActorType ActorType)
{
	const FCells Cells = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(ActorType));
	AGeneratedMap::Get().DestroyLevelActorsOnCells(Cells);
}

// Destroy characters in specified slots
void UMyCheatManager::DestroyPlayersBySlots(const FString& Slot)
{
	// Set bitmask
	const int32 Bitmask = GetBitmaskFromReverseString(Slot);
	if (!Bitmask)
	{
		return;
	}

	AGeneratedMap& GeneratedMap = AGeneratedMap::Get();

	// Get all players
	FCells CellsToDestroy;
	FMapComponents MapComponents;
	ULevelActorsUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::Player));
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		const APlayerCharacter* PlayerCharacter = MapComponentIt ? MapComponentIt->GetOwner<APlayerCharacter>() : nullptr;
		if (!PlayerCharacter)
		{
			continue;
		}

		const int32 PlayerIndex = PlayerCharacter->GetPlayerId();
		const bool bDestroy = (1 << PlayerIndex & Bitmask) != 0;
		if (bDestroy) // mark to destroy if specified in slot
		{
			CellsToDestroy.Emplace(MapComponentIt->GetCell());
		}
	}

	// Destroy all specified
	GeneratedMap.DestroyLevelActorsOnCells(CellsToDestroy);
}

/*********************************************************************************************
 * Box
 ********************************************************************************************* */

// Override the chance to spawn item after box destroying
void UMyCheatManager::SetItemChance(int32 Chance)
{
	// Get all boxes
	FMapComponents MapComponents;
	ULevelActorsUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::Box));
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		ABoxActor* BoxActor = MapComponentIt ? MapComponentIt->GetOwner<ABoxActor>() : nullptr;
		if (BoxActor)
		{
			// Override new chance
			BoxActor->SpawnItemChanceInternal = Chance;
		}
	}
}

/*********************************************************************************************
 * Player
 ********************************************************************************************* */

// Override the level of each powerup for a controlled player
void UMyCheatManager::SetPlayerPowerups(int32 NewLevel)
{
	if (APlayerCharacter* PlayerCharacter = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter())
	{
		PlayerCharacter->SetPowerups(NewLevel);
	}
}

// Enable or disable the God mode to make a controllable player undestroyable
void UMyCheatManager::SetGodMode(bool bShouldEnable)
{
	const APlayerCharacter* ControllablePlayer = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	if (UMapComponent* MapComponent = UMapComponent::GetMapComponent(ControllablePlayer))
	{
		MapComponent->SetUndestroyable(bShouldEnable);
	}
}

// Enable or disable the Auto Copilot mode to make a controllable player to play automatically
void UMyCheatManager::SetAutoCopilot()
{
	APlayerCharacter* LocalPlayer = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	if (!LocalPlayer
	    || !LocalPlayer->HasAuthority())
	{
		return;
	}

	const AMyGameModeBase* MyGameMode = UMyBlueprintFunctionLibrary::GetMyGameMode();
	checkf(MyGameMode, TEXT("ERROR: [%i] %hs:\n'MyGameMode' is null!"), __LINE__, __FUNCTION__);

	const TSubclassOf<AController> ControllerClass = LocalPlayer->IsPlayerControlled() ? LocalPlayer->AIControllerClass : MyGameMode->PlayerControllerClass;
	LocalPlayer->TryPossessController(ControllerClass);
}

/*********************************************************************************************
 * AI
 ********************************************************************************************* */

// Override the level of each powerup for bots
void UMyCheatManager::SetAIPowerups(int32 NewLevel)
{
	// Get all players
	FMapComponents MapComponents;
	ULevelActorsUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::Player));

	// Override the level of each powerup for bots
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		APlayerCharacter* Character = MapComponentIt ? MapComponentIt->GetOwner<APlayerCharacter>() : nullptr;
		if (Character
		    && Character->IsBotControlled())
		{
			Character->SetPowerups(NewLevel);
		}
	}
}

/*********************************************************************************************
 * Debug
 ********************************************************************************************* */

// Shows coordinates of all level actors by specified types
void UMyCheatManager::DisplayCells(const FString& ActorTypesString)
{
	// Set on the level to visualize new level actors
	const int32 ActorTypesBitmask = GetBitmaskFromActorTypesString(ActorTypesString);
	AGeneratedMap::Get().SetDisplayCellsActorTypes(ActorTypesBitmask);

	// Update existed level actors
	FMapComponents MapComponents;
	ULevelActorsUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::All));
	for (UMapComponent* MapComponentIt : MapComponents)
	{
		// Clear previous cell renders for all level actors in game
		UCellsUtilsLibrary::ClearDisplayedCells(MapComponentIt);

		// Show new cell renders for specified level actors
		MapComponentIt->TryDisplayOwnedCell();
	}
}

/*********************************************************************************************
 * Level
 ********************************************************************************************* */

// Sets the size for generated map, it will automatically regenerate the level for given size
void UMyCheatManager::SetLevelSize(const FString& LevelSize)
{
	static const FString Delimiter = TEXT("x");
	FString Width = TEXT("");
	FString Height = TEXT("");

	if (LevelSize.Split(Delimiter, &Width, &Height, ESearchCase::IgnoreCase))
	{
		const FIntPoint NewLevelSize(FCString::Atoi(*Width), FCString::Atoi(*Height));
		AGeneratedMap::Get().SetLevelSize(NewLevelSize);
	}
}

/*********************************************************************************************
 * Camera
 ********************************************************************************************* */

// Is overridden to let internal systems know that camera manager is enabled
void UMyCheatManager::EnableDebugCamera()
{
	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	if (ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPC' is null!"), __LINE__, __FUNCTION__))
	{
		// Enable the Debug Camera as early as possible, so during Super call it will be already enabled
		MyPC->bIsDebugCameraEnabledInternal = true;
	}

	Super::EnableDebugCamera();
}

// Is overridden to let internal systems know that camera manager is disabled
void UMyCheatManager::DisableDebugCamera()
{
	Super::DisableDebugCamera();

	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	if (ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPC' is null!"), __LINE__, __FUNCTION__))
	{
		// Disable debug camera as late as possible, so during Super call it will be still enabled
		MyPC->bIsDebugCameraEnabledInternal = false;
		MyPC->ApplyAllInputContexts();
	}
}

// Tweak the custom additive angle to affect the fit distance calculation from camera to the level
void UMyCheatManager::FitViewAdditiveAngle(float InFitViewAdditiveAngle)
{
	if (UMyCameraComponent* LevelCamera = UMyBlueprintFunctionLibrary::GetLevelCamera())
	{
		FCameraDistanceParams DistanceParams = LevelCamera->GetCameraDistanceParams();
		DistanceParams.FitViewAdditiveAngle = InFitViewAdditiveAngle;
		LevelCamera->SetCameraDistanceParams(DistanceParams);
	}
}

// Tweak the minimal distance in UU from camera to the level
void UMyCheatManager::MinDistance(float InMinDistance)
{
	if (UMyCameraComponent* LevelCamera = UMyBlueprintFunctionLibrary::GetLevelCamera())
	{
		FCameraDistanceParams DistanceParams = LevelCamera->GetCameraDistanceParams();
		DistanceParams.MinDistance = InMinDistance;
		LevelCamera->SetCameraDistanceParams(DistanceParams);
	}
}