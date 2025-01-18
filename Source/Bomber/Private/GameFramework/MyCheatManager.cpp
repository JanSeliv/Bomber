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
#include "DataAssets/PlayerDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/WidgetsSubsystem.h"
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
	APlayerCharacter* DestroyCauser = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	GeneratedMap.DestroyLevelActorsOnCells(CellsToDestroy, DestroyCauser);
}

/*********************************************************************************************
 * Box
 ********************************************************************************************* */

// Override the percentage of items spawn from boxes
TAutoConsoleVariable<int32> UMyCheatManager::CVarPowerupsChance(
	TEXT("Bomber.Box.SetPowerupsChance"),
	0.f,
	TEXT("100 - is maximum, 0 - is disabled (default chance will be used)"),
	ECVF_Cheat);

/*********************************************************************************************
 * Bomb
 ********************************************************************************************* */

// Override blast radius of all bombs
TAutoConsoleVariable<int32> UMyCheatManager::CVarBombRadius(
	TEXT("Bomber.Bomb.SetRadius"),
	INDEX_NONE,
	TEXT("5 - Set five cells blast radius to each side of all bombs"),
	ECVF_Cheat);

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

// Enable or disable the Auto Copilot mode to make a controllable player to play automatically
void UMyCheatManager::SetAutoCopilot()
{
	APlayerCharacter* LocalPlayer = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	APlayerState* PlayerState = LocalPlayer ? LocalPlayer->GetPlayerState() : nullptr;
	if (!PlayerState)
	{
		return;
	}

	// Toggle the Copilot mode
	PlayerState->SetIsABot(!PlayerState->IsABot());
	LocalPlayer->TryPossessController();
}

/*********************************************************************************************
 * AI
 ********************************************************************************************* */

// Enable or disable all bots
TAutoConsoleVariable<bool> UMyCheatManager::CVarAISetEnabled(
	TEXT("Bomber.AI.SetEnabled"),
	true,
	TEXT("Enable or disable all bots: 1 (Enable) OR 0 (Disable)"),
	ECVF_Cheat);

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

// If called, all bots will change own skin to look like players
void UMyCheatManager::ApplyPlayersSkinOnAI()
{
	// Get all players
	FMapComponents MapComponents;
	ULevelActorsUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::Player));

	// Apply players skin on AI
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		APlayerCharacter* Character = MapComponentIt ? MapComponentIt->GetOwner<APlayerCharacter>() : nullptr;
		if (Character
		    && Character->IsBotControlled())
		{
			constexpr bool bForcePlayerSkin = true;
			Character->SetDefaultPlayerMeshData(bForcePlayerSkin);
		}
	}
}

// Spawns additional bots at the center of the level
void UMyCheatManager::AddBot()
{
	const FIntPoint CenterPosition = UCellsUtilsLibrary::GetCenterCellPositionOnLevel();
	const int32 LastRowIndex = UPlayerDataAsset::Get().GetRowsNum() - 1;
	SpawnActorByType(EActorType::Player, CenterPosition.X, CenterPosition.Y, LastRowIndex);
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

	if (!LevelSize.Split(Delimiter, &Width, &Height, ESearchCase::IgnoreCase))
	{
		return;
	}

	// Restart the level
	AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
	if (AMyGameStateBase::GetCurrentGameState() == ECGS::InGame)
	{
		MyGameState->SetGameState(ECurrentGameState::GameStarting);
	}

	// Update the level size
	const FIntPoint NewLevelSize(FCString::Atoi(*Width), FCString::Atoi(*Height));
	AGeneratedMap::Get().SetLevelSize(NewLevelSize);
}

// Spawns an actor by type on the level
void UMyCheatManager::SpawnActorByType(EActorType ActorType, int32 ColumnX, int32 RowY, int32 RowIndex)
{
	AGeneratedMap& GeneratedMap = AGeneratedMap::Get();
	const FCell Cell = UCellsUtilsLibrary::GetCellByPositionOnLevel(ColumnX, RowY);

	// Destroy existed actor on the cell
	GeneratedMap.DestroyLevelActorsOnCells({Cell});

	// Spawn new actor on the cell
	const TFunction<void(AActor*)>& OnSpawned = [RowIndex](const AActor* SpawnedActor)
	{
		UMapComponent* MapComponent = UMapComponent::GetMapComponent(SpawnedActor);
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);

		const ULevelActorRow* Row = MapComponent->GetActorDataAssetChecked().GetRowByIndex(RowIndex);
		if (ensureMsgf(Row, TEXT("ASSERT: [%i] %hs:\n'Row' was not found by '%i' index!"), __LINE__, __FUNCTION__, RowIndex))
		{
			MapComponent->SetMesh(Row->Mesh);
		}
	};
	GeneratedMap.SpawnActorByType(ActorType, Cell, OnSpawned);
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

/*********************************************************************************************
 * UI
 ********************************************************************************************* */

// Completely removes all widgets from UI
void UMyCheatManager::SetUIHideAllWidgets()
{
	UWidgetsSubsystem* WidgetsSubsystem = UWidgetsSubsystem::GetWidgetsSubsystem();
	if (ensureMsgf(WidgetsSubsystem, TEXT("ASSERT: [%i] %hs:\n'WidgetsSubsystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		const bool bMakeVisibleInversed = WidgetsSubsystem->AreAllWidgetsHidden();
		WidgetsSubsystem->SetAllWidgetsVisibility(bMakeVisibleInversed);
	}
}

/*********************************************************************************************
 * Game
 ********************************************************************************************* */

// Sets current game state to the specified one
void UMyCheatManager::SetGameState(ECurrentGameState GameState)
{
	AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
	if (!MyGameState
	    || !MyGameState->HasAuthority())
	{
		return;
	}

	// Hardcoding the game state values for the state transitions is necessary because:
	// - We cannot rely on iterating enums using TEnumRange or bitmasks as enum members may not be defined in a logical order.
	// - This ensures deterministic transitions and avoids potential issues caused by chaotic ordering of enum members.
	static const TArray<ECurrentGameState> GameStateOrder = {
		ECurrentGameState::Menu,
		ECurrentGameState::GameStarting,
		ECurrentGameState::InGame,
		ECurrentGameState::EndGame
	};

	// Find the current position and target position in the transition order
	const ECurrentGameState CurrentState = MyGameState->GetCurrentGameState();
	const int32 CurrentIndex = GameStateOrder.IndexOfByKey(CurrentState);
	const int32 TargetIndex = GameStateOrder.IndexOfByKey(GameState);

	// Execute immediately if the target state is not part of the standard order, in case if some state is part of additional modes
	if (TargetIndex == INDEX_NONE)
	{
		MyGameState->SetGameState(GameState);
		return;
	}

	// Start iterating from the beginning if the target is before the current state, otherwise continue from the current state
	constexpr int32 InitialIndex = 0;
	int32 StartIndex = TargetIndex < CurrentIndex ? InitialIndex : CurrentIndex;
	StartIndex = FMath::Max(InitialIndex, StartIndex);
	for (int32 Index = StartIndex; Index <= TargetIndex; ++Index)
	{
		MyGameState->SetGameState(GameStateOrder[Index]);
	}
}