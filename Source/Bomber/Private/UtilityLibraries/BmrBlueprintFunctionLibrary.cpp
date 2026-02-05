// Copyright (c) Yevhenii Selivanov.

#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "Components/BmrMouseActivityComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "DataAssets/BmrDataAssetsContainer.h"
#include "DataAssets/BmrLevelActorDataAsset.h"
#include "Engine/BmrGameViewportClient.h"
#include "GameFramework/BmrGameMode.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrGameUserSettings.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "Subsystems/BmrGeneratedMapSubsystem.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UI/Widgets/BmrHUDWidget.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"

// UE
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBlueprintFunctionLibrary)

/* ---------------------------------------------------
 *		Static library functions
 * --------------------------------------------------- */

// Returns number of alive players
int32 UBmrBlueprintFunctionLibrary::GetAlivePlayersNum(EBmrPlayerType InPlayerType)
{
	FMapComponents AllPlayers;
	UBmrActorUtilsLibrary::GetLevelActors(/*out*/ AllPlayers, TO_FLAG(EBmrActorType::Player));

	int32 PlayersNum = 0;
	for (const UBmrMapComponent* MapComponentIt : AllPlayers)
	{
		const ABmrPawn* PlayerChar = MapComponentIt ? MapComponentIt->GetOwner<ABmrPawn>() : nullptr;
		const ABmrPlayerState* PlayerState = PlayerChar ? PlayerChar->GetPlayerState<ABmrPlayerState>() : nullptr;
		if (!PlayerState)
		{
			continue;
		}

		const EBmrPlayerType PlayerTypeIt = PlayerState->GetPlayerType();
		if (InPlayerType == EBmrPlayerType::Any
		    || InPlayerType == PlayerTypeIt)
		{
			++PlayersNum;
		}
	}

	return PlayersNum;
}

// Returns the type of the current level
EBmrLevelType UBmrBlueprintFunctionLibrary::GetLevelType()
{
	// @TODO JanSeliv StB8orDX: remove level type enum and replace related logic
	return EBmrLevelType::First;
}

// Returns true if the local pawn is ready (spawned, possessed, and replicated)
bool UBmrBlueprintFunctionLibrary::IsLocalPawnReady(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrPawn* LocalPawn = GetLocalPawn(OptionalWorldContext);
	return UBmrGameplayMessageSubsystem::Get(OptionalWorldContext).ReadyHandler.IsReady(LocalPawn);
}

/* ---------------------------------------------------
 *		Framework pointer getters
 * --------------------------------------------------- */

// Contains a data of Bomber Level, nullptr otherwise
ABmrGameMode* UBmrBlueprintFunctionLibrary::GetGameMode(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetAuthGameMode<ABmrGameMode>() : nullptr;
}

// Returns the Bomber Game state, nullptr otherwise.
ABmrGameState* UBmrBlueprintFunctionLibrary::GetGameState(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetGameState<ABmrGameState>() : nullptr;
}

// Returns the Bomber Player Controller, nullptr otherwise
ABmrPlayerController* UBmrBlueprintFunctionLibrary::GetPlayerController(int32 PlayerIndex, const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrGameMode* MyGameMode = GetGameMode(OptionalWorldContext);
	ABmrPlayerController* MyPC = MyGameMode ? MyGameMode->GetPlayerController(PlayerIndex) : nullptr;
	if (MyPC)
	{
		return MyPC;
	}

	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return Cast<ABmrPlayerController>(UGameplayStatics::GetPlayerController(World, PlayerIndex));
}

// Returns the local Player Controller, nullptr otherwise
ABmrPlayerController* UBmrBlueprintFunctionLibrary::GetLocalPlayerController(const UObject* OptionalWorldContext /* = nullptr*/)
{
	static constexpr int32 LocalPlayerIndex = 0;
	return GetPlayerController(LocalPlayerIndex, OptionalWorldContext);
}

// Returns the Bomber Player State for specified player, nullptr otherwise
ABmrPlayerState* UBmrBlueprintFunctionLibrary::GetPlayerState(int32 PlayerId, const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrPawn* PlayerChar = GetPawn(PlayerId, OptionalWorldContext);
	return PlayerChar ? PlayerChar->GetPlayerState<ABmrPlayerState>() : nullptr;
}

// Returns the player state of current controller
ABmrPlayerState* UBmrBlueprintFunctionLibrary::GetLocalPlayerState(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrPlayerController* MyPlayerController = Cast<ABmrPlayerController>(OptionalWorldContext);
	if (!MyPlayerController)
	{
		MyPlayerController = GetLocalPlayerController(OptionalWorldContext);
		if (!MyPlayerController)
		{
			return nullptr;
		}
	}

	if (ABmrPlayerState* PlayerState = MyPlayerController->GetPlayerState<ABmrPlayerState>())
	{
		return PlayerState;
	}

	if (const ABmrGameMode* MyGameMode = GetGameMode(OptionalWorldContext))
	{
		const int32 PCIndex = MyGameMode->GetPlayerControllerIndex(MyPlayerController);
		return GetPlayerState(PCIndex);
	}

	return nullptr;
}

// Returns the Bomber settings
UBmrGameUserSettings* UBmrBlueprintFunctionLibrary::GetGameUserSettings(const UObject* OptionalWorldContext /* = nullptr*/)
{
	return GEngine ? Cast<UBmrGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

// Returns the settings widget
USettingsWidget* UBmrBlueprintFunctionLibrary::GetSettingsWidget(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem(OptionalWorldContext);
	return WidgetsSubsystem ? WidgetsSubsystem->GetWidgetByTag<USettingsWidget>(BmrGameplayTags::UI::Widget_Settings) : nullptr;
}

// Returns the HUD widget
class UBmrHUDWidget* UBmrBlueprintFunctionLibrary::GetHUDWidget(const UObject* OptionalWorldContext)
{
	const UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem(OptionalWorldContext);
	return WidgetsSubsystem ? WidgetsSubsystem->GetWidgetByTag<UBmrHUDWidget>(BmrGameplayTags::UI::Widget_HUD) : nullptr;
}

// Returns the Camera Component used on level
UBmrCameraComponent* UBmrBlueprintFunctionLibrary::GetLevelCamera(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const UBmrGeneratedMapSubsystem* Subsystem = UBmrGeneratedMapSubsystem::GetGeneratedMapSubsystem(OptionalWorldContext);
	const ABmrGeneratedMap* GeneratedMap = Subsystem ? Subsystem->GetGeneratedMap() : nullptr;
	return GeneratedMap ? GeneratedMap->GetCameraComponent() : nullptr;
}

// Returns specified player character, by default returns local player
ABmrPawn* UBmrBlueprintFunctionLibrary::GetPawn(int32 PlayerId, const UObject* OptionalWorldContext /* = nullptr*/)
{
	if (PlayerId < 0)
	{
		// No ID is set, return local player character
		return GetLocalPawn(OptionalWorldContext);
	}

	const UBmrMapComponent* PlayerMapComponent = UBmrActorUtilsLibrary::GetLevelActorByIndex(PlayerId, TO_FLAG(EAT::Player));
	return PlayerMapComponent ? PlayerMapComponent->GetOwner<ABmrPawn>() : nullptr;
}

// Returns controlled player character
ABmrPawn* UBmrBlueprintFunctionLibrary::GetLocalPawn(const UObject* OptionalWorldContext /* = nullptr*/)
{
	static constexpr int32 LocalPlayerIndex = 0;
	const ABmrPlayerController* MyPC = GetPlayerController(LocalPlayerIndex, OptionalWorldContext);
	ABmrPawn* LocalPlayer = MyPC ? MyPC->GetPawn<ABmrPawn>() : nullptr;

	if (!LocalPlayer)
	{
		// In some edge cases, character is not possessed by the controller, try get it from the map
		LocalPlayer = GetPawn(LocalPlayerIndex, OptionalWorldContext);
	}

	return LocalPlayer;
}

// Returns specified Ability System Component
class UAbilitySystemComponent* UBmrBlueprintFunctionLibrary::GetAbilitySystemComponent(int32 PlayerId, const UObject* OptionalWorldContext)
{
	const ABmrPlayerState* PlayerState = GetPlayerState(PlayerId, OptionalWorldContext);
	return PlayerState ? PlayerState->GetAbilitySystemComponent() : nullptr;
}

// Returns the Ability System Component from the local Player State
class UAbilitySystemComponent* UBmrBlueprintFunctionLibrary::GetLocalAbilitySystemComponent(const UObject* OptionalWorldContext)
{
	const ABmrPlayerState* PlayerState = GetLocalPlayerState(OptionalWorldContext);
	return PlayerState ? PlayerState->GetAbilitySystemComponent() : nullptr;
}

// Returns specified Mover Component
class UBmrMoverComponent* UBmrBlueprintFunctionLibrary::GetMoverComponent(int32 PlayerId, const UObject* OptionalWorldContext)
{
	const ABmrPawn* Pawn = GetPawn(PlayerId, OptionalWorldContext);
	return Pawn ? Pawn->GetMoverComponent() : nullptr;
}

// Returns the Mover Component from the local Player Character
class UBmrMoverComponent* UBmrBlueprintFunctionLibrary::GetLocalMoverComponent(const UObject* OptionalWorldContext)
{
	const ABmrPawn* Pawn = GetLocalPawn(OptionalWorldContext);
	return Pawn ? Pawn->GetMoverComponent() : nullptr;
}

// Returns implemented Game Viewport Client on the project side
UBmrGameViewportClient* UBmrBlueprintFunctionLibrary::GetGameViewportClient()
{
	return GEngine ? Cast<UBmrGameViewportClient>(GEngine->GameViewport) : nullptr;
}

// Returns the component that responsible for mouse-related logic like showing and hiding itself
UBmrMouseActivityComponent* UBmrBlueprintFunctionLibrary::GetMouseActivityComponent(const UObject* OptionalWorldContext /* = nullptr*/)
{
	const ABmrPlayerController* MyPC = GetLocalPlayerController(OptionalWorldContext);
	return MyPC ? MyPC->GetMouseActivityComponent() : nullptr;
}

/* ---------------------------------------------------
 *		EBmrActorType functions
 * --------------------------------------------------- */

// Returns Actor Type of specified actor, None is not level actor
EBmrActorType UBmrBlueprintFunctionLibrary::GetActorType(const AActor* Actor)
{
	const TSubclassOf<AActor> ActorClass = Actor ? Actor->GetClass() : nullptr;
	const UBmrLevelActorDataAsset* LevelActorDataAsset = ActorClass ? UBmrDataAssetsContainer::GetDataAssetByActorClass(ActorClass) : nullptr;
	return LevelActorDataAsset ? LevelActorDataAsset->GetActorType() : EAT::None;
}

// Returns true if specified actor is the Bomber Level Actor (player, box, wall or powerup)
bool UBmrBlueprintFunctionLibrary::IsLevelActor(const AActor* Actor)
{
	return GetActorType(Actor) != EAT::None;
}

// Returns true if specified level actor has at least one specified type
bool UBmrBlueprintFunctionLibrary::IsActorHasAnyMatchingType(const AActor* Actor, int32 ActorsTypesBitmask)
{
	const EBmrActorType ActorType = GetActorType(Actor);
	return BitwiseActorTypes(TO_FLAG(ActorType), ActorsTypesBitmask);
}