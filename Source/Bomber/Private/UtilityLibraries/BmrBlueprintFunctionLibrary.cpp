// Copyright (c) Yevhenii Selivanov.

#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "Components/BmrMouseActivityComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "DalSubsystem.h"
#include "DalUtilsLibrary.h"
#include "DataAssets/BmrLevelActorDataAsset.h"
#include "Engine/BmrGameViewportClient.h"
#include "GameFramework/BmrGameMode.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrGameUserSettings.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrPawnReadySubsystem.h"
#include "Subsystems/BmrGeneratedMapSubsystem.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UI/Widgets/BmrHUDWidget.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"

// UE
#include "Engine/AssetManager.h"
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
	return UBmrPawnReadySubsystem::Get(OptionalWorldContext).IsReady(LocalPawn);
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

// Returns the world-level Ability System Component used for environmental abilities and game-wide tag management
class UAbilitySystemComponent* UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent(const UObject* OptionalWorldContext)
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap(OptionalWorldContext);
	return GeneratedMap ? GeneratedMap->GetAbilitySystemComponent() : nullptr;
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

// Returns true if specified actor is the Bomber Level Actor (player, box, wall or powerup)
bool UBmrBlueprintFunctionLibrary::IsLevelActor(const AActor* Actor)
{
	return Actor && GetActorTypeByLevelActor(Actor) != EAT::None;
}

// Returns true if specified level actor has at least one specified type
bool UBmrBlueprintFunctionLibrary::IsActorHasAnyMatchingType(const AActor* Actor, int32 ActorsTypesBitmask)
{
	const EBmrActorType ActorType = GetActorTypeByLevelActor(Actor);
	return BitwiseActorTypes(TO_FLAG(ActorType), ActorsTypesBitmask);
}

// Returns the actor type for the specified actor, obtaining it from asset registry keys without loading any dependencies, EBmrActorType::None if not found
EBmrActorType UBmrBlueprintFunctionLibrary::GetActorTypeByLevelActor(const AActor* Actor)
{
	const UClass* ActorClass = Actor ? Actor->GetClass() : nullptr;
	if (!ActorClass)
	{
		return EAT::None;
	}

	const FAssetData FoundAsset = UDalUtilsLibrary::GetAssetByRegistryTag(UBmrLevelActorDataAsset::ActorClassTag, GetPathNameSafe(ActorClass));
	if (!FoundAsset.IsValid())
	{
		return EAT::None;
	}

	const FString ActorTypeStr = FoundAsset.GetTagValueRef<FString>(UBmrLevelActorDataAsset::ActorTypeTag);
	const EBmrActorType ActorType = TO_ENUM(EBmrActorType, StaticEnum<EBmrActorType>()->GetValueByNameString(ActorTypeStr));
	return ActorType;
}

// Returns the level actor data asset associated with the specified level actor
const UBmrLevelActorDataAsset* UBmrBlueprintFunctionLibrary::GetDataAssetByLevelActor(const AActor* Actor)
{
	const TSubclassOf<UBmrLevelActorDataAsset> DataAssetClass = Actor ? GetDataAssetClassByActorClass(Actor->GetClass()) : nullptr;
	return UDalSubsystem::Get().GetDataAsset<UBmrLevelActorDataAsset>(DataAssetClass);
}

// Returns the first found level actor data asset matching the specified actor type
const UBmrLevelActorDataAsset* UBmrBlueprintFunctionLibrary::GetDataAssetByActorType(EBmrActorType ActorType)
{
	const TSubclassOf<UBmrLevelActorDataAsset> DataAssetClass = GetDataAssetClassByActorType(ActorType);
	return UDalSubsystem::Get().GetDataAsset<UBmrLevelActorDataAsset>(DataAssetClass);
}

// Returns the data asset class for the specified level actor from asset registry tags without loading
TSubclassOf<UBmrLevelActorDataAsset> UBmrBlueprintFunctionLibrary::GetDataAssetClassByActorClass(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	const FAssetData FoundAsset = UDalUtilsLibrary::GetAssetByRegistryTag(UBmrLevelActorDataAsset::ActorClassTag, GetPathNameSafe(ActorClass));
	if (!FoundAsset.IsValid())
	{
		return nullptr;
	}

	const FSoftClassPath DataAssetClassPath(FoundAsset.AssetClassPath.ToString());
	return DataAssetClassPath.ResolveClass();
}

// Returns the data asset class for the specified level actor type from asset registry tags without loading
TSubclassOf<class UBmrLevelActorDataAsset> UBmrBlueprintFunctionLibrary::GetDataAssetClassByActorType(EBmrActorType ActorType)
{
	const FString ActorTypeStr = ActorType != EAT::None ? UEnum::GetValueAsString(ActorType) : FString();
	const FAssetData FoundAsset = UDalUtilsLibrary::GetAssetByRegistryTag(UBmrLevelActorDataAsset::ActorTypeTag, ActorTypeStr);
	if (!FoundAsset.IsValid())
	{
		return nullptr;
	}

	const FSoftClassPath DataAssetClassPath(FoundAsset.AssetClassPath.ToString());
	return DataAssetClassPath.ResolveClass();
}

// Returns the actor class associated with the specified actor type, obtaining it from asset registry keys without loading any dependencies, nullptr if not found
const UClass* UBmrBlueprintFunctionLibrary::GetActorClassByActorType(EBmrActorType ActorType)
{
	const FString ActorTypeStr = ActorType != EAT::None ? UEnum::GetValueAsString(ActorType) : FString();
	const FAssetData FoundAsset = UDalUtilsLibrary::GetAssetByRegistryTag(UBmrLevelActorDataAsset::ActorTypeTag, ActorTypeStr);
	if (!FoundAsset.IsValid())
	{
		return nullptr;
	}

	const FSoftClassPath ActorClass(FoundAsset.GetTagValueRef<FString>(UBmrLevelActorDataAsset::ActorClassTag));
	return ActorClass.ResolveClass();
}

// Returns the data asset classes for the specified actor types from asset registry tags without loading
void UBmrBlueprintFunctionLibrary::GetDataAssetsByActorTypes(TArray<TSubclassOf<UDalPrimaryDataAsset>>& OutDataAssetClasses, int32 ActorsTypesBitmask)
{
	TMultiMap<FName, TOptional<FString>> TagsAndValues;
	for (int32 It = 1; It < TO_FLAG(EBmrActorType::All); It <<= 1)
	{
		if (BitwiseActorTypes(It, ActorsTypesBitmask))
		{
			const EBmrActorType ActorType = TO_ENUM(EBmrActorType, It);
			TagsAndValues.Add(UBmrLevelActorDataAsset::ActorTypeTag, UEnum::GetValueAsString(ActorType));
		}
	}

	TArray<FAssetData> AssetsData;
	UDalUtilsLibrary::GetAssetsByRegistryTags(/*out*/ AssetsData, TagsAndValues);
	for (const FAssetData& AssetDataIt : AssetsData)
	{
		const FSoftClassPath DataAssetClassPath(AssetDataIt.AssetClassPath.ToString());
		if (UClass* DataAssetClass = DataAssetClassPath.ResolveClass())
		{
			OutDataAssetClasses.Add(DataAssetClass);
		}
	}
}
