// Copyright (c) Yevhenii Selivanov.

#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MouseActivityComponent.h"
#include "Controllers/MyPlayerController.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/LevelActorDataAsset.h"
#include "Engine/MyGameViewportClient.h"
#include "GameFramework/MyGameModeBase.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyGameUserSettings.h"
#include "GameFramework/MyPlayerState.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GeneratedMapSubsystem.h"
#include "UI/InGameMenuWidget.h"
#include "UI/InGameWidget.h"
#include "UI/MyHUD.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyBlueprintFunctionLibrary)

/* ---------------------------------------------------
 *		Static library functions
 * --------------------------------------------------- */

// Returns number of alive players
int32 UMyBlueprintFunctionLibrary::GetAlivePlayersNum()
{
	return UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Player)).Num();
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
AMyGameModeBase* UMyBlueprintFunctionLibrary::GetMyGameMode(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetAuthGameMode<AMyGameModeBase>() : nullptr;
}

// Returns the Bomber Game state, nullptr otherwise.
AMyGameStateBase* UMyBlueprintFunctionLibrary::GetMyGameState(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetGameState<AMyGameStateBase>() : nullptr;
}

// Returns the Bomber Player Controller, nullptr otherwise
AMyPlayerController* UMyBlueprintFunctionLibrary::GetMyPlayerController(int32 PlayerIndex, const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyGameModeBase* MyGameMode = GetMyGameMode(OptionalWorldContext);
	AMyPlayerController* MyPC = MyGameMode ? MyGameMode->GetPlayerController(PlayerIndex) : nullptr;
	if (MyPC)
	{
		return MyPC;
	}

	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(World, PlayerIndex));
}

// Returns the local Player Controller, nullptr otherwise
AMyPlayerController* UMyBlueprintFunctionLibrary::GetLocalPlayerController(const UObject* OptionalWorldContext/* = nullptr*/)
{
	static constexpr int32 LocalPlayerIndex = 0;
	return GetMyPlayerController(LocalPlayerIndex, OptionalWorldContext);
}

// Returns the Bomber Player State for specified player, nullptr otherwise
AMyPlayerState* UMyBlueprintFunctionLibrary::GetMyPlayerState(const APawn* Pawn)
{
	return Pawn ? Pawn->GetPlayerState<AMyPlayerState>() : nullptr;
}

// Returns the player state of current controller
AMyPlayerState* UMyBlueprintFunctionLibrary::GetLocalPlayerState(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyPlayerController* MyPlayerController = GetLocalPlayerController(OptionalWorldContext);
	return MyPlayerController ? MyPlayerController->GetPlayerState<AMyPlayerState>() : nullptr;
}

// Returns the Bomber settings
UMyGameUserSettings* UMyBlueprintFunctionLibrary::GetMyGameUserSettings(const UObject* OptionalWorldContext/* = nullptr*/)
{
	return GEngine ? Cast<UMyGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

// Returns the settings widget
USettingsWidget* UMyBlueprintFunctionLibrary::GetSettingsWidget(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyHUD* MyHUD = GetMyHUD(OptionalWorldContext);
	return MyHUD ? MyHUD->GetSettingsWidget() : nullptr;
}

// Returns the Camera Component used on level
UMyCameraComponent* UMyBlueprintFunctionLibrary::GetLevelCamera(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UGeneratedMapSubsystem* Subsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem(OptionalWorldContext);
	const AGeneratedMap* GeneratedMap = Subsystem ? Subsystem->GetGeneratedMap() : nullptr;
	return GeneratedMap ? GeneratedMap->GetCameraComponent() : nullptr;
}

// Returns the HUD actor
AMyHUD* UMyBlueprintFunctionLibrary::GetMyHUD(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyPlayerController* MyPlayerController = GetLocalPlayerController(OptionalWorldContext);
	return MyPlayerController ? MyPlayerController->GetHUD<AMyHUD>() : nullptr;
}

// Returns the In-Game widget
UInGameWidget* UMyBlueprintFunctionLibrary::GetInGameWidget(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyHUD* MyHUD = GetMyHUD(OptionalWorldContext);
	return MyHUD ? MyHUD->GetInGameWidget() : nullptr;
}

// Returns the In-Game Menu widget
UInGameMenuWidget* UMyBlueprintFunctionLibrary::GetInGameMenuWidget(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const UInGameWidget* InGameWidget = GetInGameWidget(OptionalWorldContext);
	return InGameWidget ? InGameWidget->GetInGameMenuWidget() : nullptr;
}

// Returns specified player character, by default returns local player
APlayerCharacter* UMyBlueprintFunctionLibrary::GetPlayerCharacter(int32 PlayerIndex, const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyPlayerController* MyPC = GetMyPlayerController(PlayerIndex, OptionalWorldContext);
	return MyPC ? MyPC->GetPawn<APlayerCharacter>() : nullptr;
}

// Returns controlled player character
APlayerCharacter* UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter(const UObject* OptionalWorldContext/* = nullptr*/)
{
	static constexpr int32 LocalPlayerIndex = 0;
	return GetPlayerCharacter(LocalPlayerIndex, OptionalWorldContext);
}

// Returns implemented Game Viewport Client on the project side
UMyGameViewportClient* UMyBlueprintFunctionLibrary::GetGameViewportClient()
{
	return GEngine ? Cast<UMyGameViewportClient>(GEngine->GameViewport) : nullptr;
}

// Returns the component that responsible for mouse-related logic like showing and hiding itself
UMouseActivityComponent* UMyBlueprintFunctionLibrary::GetMouseActivityComponent(const UObject* OptionalWorldContext/* = nullptr*/)
{
	const AMyPlayerController* MyPC = GetLocalPlayerController(OptionalWorldContext);
	return MyPC ? MyPC->GetMouseActivityComponent() : nullptr;
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
