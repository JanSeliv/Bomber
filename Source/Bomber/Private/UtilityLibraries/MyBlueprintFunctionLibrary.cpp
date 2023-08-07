// Copyright (c) Yevhenii Selivanov.

#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Controllers/MyPlayerController.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/LevelActorDataAsset.h"
#include "Engine/MyGameViewportClient.h"
#include "GameFramework/MyGameModeBase.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyGameUserSettings.h"
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/GeneratedMapSubsystem.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UI/InGameMenuWidget.h"
#include "UI/InGameWidget.h"
#include "UI/MyHUD.h"
//---
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyBlueprintFunctionLibrary)

/* ---------------------------------------------------
 *		Static library functions
 * --------------------------------------------------- */

// Returns current play world
UWorld* UMyBlueprintFunctionLibrary::GetStaticWorld()
{
	// Get the world from Generated Map
	const UGeneratedMapSubsystem* GeneratedMapSubsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem();
	return GeneratedMapSubsystem ? GeneratedMapSubsystem->GetWorld() : nullptr;
}

// Returns true if game was started
bool UMyBlueprintFunctionLibrary::HasWorldBegunPlay()
{
#if WITH_EDITOR	// [UEditorUtils::IsEditor]
	if (FEditorUtilsLibrary::IsPIE())
	{
		return true;
	}
#endif	// [UEditorUtils::IsEditor]
	const UWorld* World = GetStaticWorld();
	return World && World->HasBegunPlay();
}

// Returns true if this instance is server
bool UMyBlueprintFunctionLibrary::IsServer()
{
	const UWorld* World = GetStaticWorld();
	return World && !World->IsNetMode(NM_Client);
}

// Returns number of alive players
int32 UMyBlueprintFunctionLibrary::GetAlivePlayersNum()
{
	const AGeneratedMap* GeneratedMap = UGeneratedMapSubsystem::Get().GetGeneratedMap();
	return GeneratedMap ? GeneratedMap->GetAlivePlayersNum() : INDEX_NONE;
}

// Returns the type of the current level
ELevelType UMyBlueprintFunctionLibrary::GetLevelType()
{
	const AGeneratedMap* GeneratedMap = UGeneratedMapSubsystem::Get().GetGeneratedMap();
	return GeneratedMap ? GeneratedMap->GetLevelType() : ELevelType::None;
}

/* ---------------------------------------------------
 *		Framework pointer getters
 * --------------------------------------------------- */

// Contains a data of Bomber Level, nullptr otherwise
AMyGameModeBase* UMyBlueprintFunctionLibrary::GetMyGameMode()
{
	const UWorld* World = GetStaticWorld();
	return World ? World->GetAuthGameMode<AMyGameModeBase>() : nullptr;
}

// Returns the Bomber Game state, nullptr otherwise.
AMyGameStateBase* UMyBlueprintFunctionLibrary::GetMyGameState()
{
	const UWorld* World = GetStaticWorld();
	return World ? World->GetGameState<AMyGameStateBase>() : nullptr;
}

// Returns the Bomber Player Controller, nullptr otherwise
AMyPlayerController* UMyBlueprintFunctionLibrary::GetMyPlayerController(int32 PlayerIndex)
{
	const AMyGameModeBase* MyGameMode = GetMyGameMode();
	AMyPlayerController* MyPC = MyGameMode ? MyGameMode->GetPlayerController(PlayerIndex) : nullptr;
	if (MyPC)
	{
		return MyPC;
	}

	return Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetStaticWorld(), PlayerIndex));
}

// Returns the local Player Controller, nullptr otherwise
AMyPlayerController* UMyBlueprintFunctionLibrary::GetLocalPlayerController()
{
	static constexpr int32 LocalPlayerIndex = 0;
	return GetMyPlayerController(LocalPlayerIndex);
}

// Returns the Bomber Player State for specified player, nullptr otherwise
AMyPlayerState* UMyBlueprintFunctionLibrary::GetMyPlayerState(const APawn* Pawn)
{
	return Pawn ? Pawn->GetPlayerState<AMyPlayerState>() : nullptr;
}

// Returns the player state of current controller
AMyPlayerState* UMyBlueprintFunctionLibrary::GetLocalPlayerState()
{
	const AMyPlayerController* MyPlayerController = GetLocalPlayerController();
	return MyPlayerController ? MyPlayerController->GetPlayerState<AMyPlayerState>() : nullptr;
}

// Returns the Bomber settings
UMyGameUserSettings* UMyBlueprintFunctionLibrary::GetMyGameUserSettings()
{
	return GEngine ? Cast<UMyGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

// Returns the settings widget
USettingsWidget* UMyBlueprintFunctionLibrary::GetSettingsWidget()
{
	const AMyHUD* MyHUD = GetMyHUD();
	return MyHUD ? MyHUD->GetSettingsWidget() : nullptr;
}

// Returns the Camera Component used on level
UMyCameraComponent* UMyBlueprintFunctionLibrary::GetLevelCamera()
{
	const AGeneratedMap* GeneratedMap = UGeneratedMapSubsystem::Get().GetGeneratedMap();
	return GeneratedMap ? GeneratedMap->GetCameraComponent() : nullptr;
}

// Returns the HUD actor
AMyHUD* UMyBlueprintFunctionLibrary::GetMyHUD()
{
	const AMyPlayerController* MyPlayerController = GetLocalPlayerController();
	return MyPlayerController ? MyPlayerController->GetHUD<AMyHUD>() : nullptr;
}

// Returns the In-Game widget
UInGameWidget* UMyBlueprintFunctionLibrary::GetInGameWidget()
{
	const AMyHUD* MyHUD = GetMyHUD();
	return MyHUD ? MyHUD->GetInGameWidget() : nullptr;
}

// Returns the In-Game Menu widget
UInGameMenuWidget* UMyBlueprintFunctionLibrary::GetInGameMenuWidget()
{
	const UInGameWidget* InGameWidget = GetInGameWidget();
	return InGameWidget ? InGameWidget->GetInGameMenuWidget() : nullptr;
}

// Returns specified player character, by default returns local player
APlayerCharacter* UMyBlueprintFunctionLibrary::GetPlayerCharacter(int32 PlayerIndex)
{
	const AMyPlayerController* MyPC = GetMyPlayerController(PlayerIndex);
	return MyPC ? MyPC->GetPawn<APlayerCharacter>() : nullptr;
}

// Returns controlled player character
APlayerCharacter* UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter()
{
	static constexpr int32 LocalPlayerIndex = 0;
	return GetPlayerCharacter(LocalPlayerIndex);
}

// Returns the Sound Manager
USoundsSubsystem* UMyBlueprintFunctionLibrary::GetSoundsSubsystem()
{
	return &USoundsSubsystem::Get();
}

// Returns implemented Game Viewport Client on the project side
UMyGameViewportClient* UMyBlueprintFunctionLibrary::GetGameViewportClient()
{
	return GEngine ? Cast<UMyGameViewportClient>(GEngine->GameViewport) : nullptr;
}

/* ---------------------------------------------------
 *		EActorType functions
 * --------------------------------------------------- */

// Returns Actor Type of specified actor, None is not level actor
EActorType UMyBlueprintFunctionLibrary::GetActorType(const AActor* Actor)
{
	const TSubclassOf<AActor> ActorClass = Actor ? Actor->GetClass() : nullptr;
	const ULevelActorDataAsset* LevelActorDataAsset = ActorClass ? UDataAssetsContainer::GetDataAssetByActorClass(ActorClass) : nullptr;
	return LevelActorDataAsset ? LevelActorDataAsset->GetActorType() : EAT::None;
}

// Returns true if specified actor is the Bomber Level Actor (player, box, wall or item)
bool UMyBlueprintFunctionLibrary::IsLevelActor(const AActor* Actor)
{
	return GetActorType(Actor) != EAT::None;
}

// Returns true if specified level actor has at least one specified type
bool UMyBlueprintFunctionLibrary::IsActorHasAnyMatchingType(const AActor* Actor, int32 ActorsTypesBitmask)
{
	const EActorType ActorType = GetActorType(Actor);
	return BitwiseActorTypes(TO_FLAG(ActorType), ActorsTypesBitmask);
}
