// Copyright (c) Yevhenii Selivanov.

#include "Globals/SingletonLibrary.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "SoundsManager.h"
#include "Components/MapComponent.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameModeBase.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyGameUserSettings.h"
#include "GameFramework/MyPlayerState.h"
#include "Globals/LevelActorDataAsset.h"
#include "Globals/MyGameInstance.h"
#include "LevelActors/PlayerCharacter.h"
#include "UI/MyHUD.h"
#include "UI/InGameWidget.h"
#include "UI/InGameMenuWidget.h"
//---
#include "Engine.h"
#include "PoolManager.h"
#include "Components/TextRenderComponent.h"
#include "Globals/DataAssetsContainer.h"
#include "Kismet/GameplayStatics.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h"
#include "Editor.h"	 // GEditor
#include "MyUnrealEdEngine.h" // GetClientSingleton
#endif

// Binds to update movements of each AI controller.
USingletonLibrary::FUpdateAI USingletonLibrary::GOnAIUpdatedDelegate;

// Default params to display cells
const FDisplayCellsParams FDisplayCellsParams::EmptyParams = FDisplayCellsParams();

// Returns a world of stored level map
UWorld* USingletonLibrary::GetWorld() const
{
#if WITH_EDITOR	 // [UEditorUtils::IsEditorNotPieWorld]
	if (UEditorUtilsLibrary::IsEditor()
	    && !Get().LevelMapInternal.IsValid())
	{
		if (UEditorUtilsLibrary::IsEditorNotPieWorld())
		{
			return GEditor->GetEditorWorldContext().World();
		}
		if (UEditorUtilsLibrary::IsPIE())
		{
			return GEditor->GetCurrentPlayWorld();
		}
	}
#endif	// WITH_EDITOR [UEditorUtils::IsEditorNotPieWorld]
	const AGeneratedMap* LevelMap = GetLevelMap();
	return LevelMap ? LevelMap->GetWorld() : nullptr;
}

/* ---------------------------------------------------
 *		Static library functions
 * --------------------------------------------------- */

//  Returns the singleton, nullptr otherwise
USingletonLibrary* USingletonLibrary::GetSingleton()
{
#if WITH_EDITOR // [UEditorUtils::IsEditorMultiplayer]
	const int32 EditorPlayerIndex = UEditorUtilsLibrary::GetEditorPlayerIndex();
	if (EditorPlayerIndex > 0)
	{
		const int32 ClientSingletonIndex = EditorPlayerIndex - 1;
		USingletonLibrary* ClientSingleton = UMyUnrealEdEngine::GetClientSingleton<USingletonLibrary>(ClientSingletonIndex);
		checkf(ClientSingleton, TEXT("The client Singleton is null"));
		return ClientSingleton;
	}
#endif // [UEditorUtils::IsEditorMultiplayer]

	USingletonLibrary* Singleton = GEngine ? Cast<USingletonLibrary>(GEngine->GameSingleton) : nullptr;
	checkf(Singleton, TEXT("The Singleton is null"));
	return Singleton;
}

// Returns true if specified actor is the Bomber Level Actor (player, box, wall or item)
bool USingletonLibrary::IsLevelActor(const AActor* Actor)
{
	const TSubclassOf<AActor> ActorClass = Actor ? Actor->GetClass() : nullptr;
	const ULevelActorDataAsset* LevelActorDataAsset = ActorClass ? UDataAssetsContainer::GetDataAssetByActorClass(ActorClass) : nullptr;
	return LevelActorDataAsset && LevelActorDataAsset->GetActorType() != EAT::None;
}

// Iterates the current world to find an actor by specified class
AActor* USingletonLibrary::GetActorOfClass(TSubclassOf<AActor> ActorClass)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(&Get(), ActorClass, FoundActors);

	static constexpr int32 Index = 0;
	return FoundActors.IsValidIndex(Index) ? FoundActors[Index] : nullptr;
}

// Returns true if game was started
bool USingletonLibrary::HasWorldBegunPlay()
{
#if WITH_EDITOR	// [UEditorUtils::IsEditor]
	if (UEditorUtilsLibrary::IsEditor())
	{
		return UEditorUtilsLibrary::IsPIE();
	}
#endif	// [UEditorUtils::IsEditor]
	const UWorld* World = Get().GetWorld();
	return World && World->HasBegunPlay();
}

// Returns true if this instance is server
bool USingletonLibrary::IsServer()
{
	const UWorld* World = Get().GetWorld();
	return World && !World->IsNetMode(ENetMode::NM_Client);
}

// The Level Map setter
void USingletonLibrary::SetLevelMap(AGeneratedMap* LevelMap)
{
	USingletonLibrary* Singleton = GetSingleton();
	if (Singleton
	    && IS_VALID(LevelMap))
	{
		Singleton->LevelMapInternal = LevelMap;
	}
}

// Returns number of alive players
int32 USingletonLibrary::GetAlivePlayersNum()
{
	return AGeneratedMap::Get().GetAlivePlayersNum();
}

// Returns the type of the current level
ELevelType USingletonLibrary::GetLevelType()
{
	return AGeneratedMap::Get().GetLevelType();
}

/* ---------------------------------------------------
 *		Framework pointer getters
 * --------------------------------------------------- */

// The Level Map getter, nullptr otherwise
AGeneratedMap* USingletonLibrary::GetLevelMap()
{
#if WITH_EDITOR	 // [UEditorUtils::IsEditorNotPieWorld]
	if (UEditorUtilsLibrary::IsEditor()
	    && !Get().LevelMapInternal.IsValid())
	{
		AGeneratedMap* LevelMap = GetActorOfClass<AGeneratedMap>(AGeneratedMap::StaticClass());
		SetLevelMap(LevelMap);
	}
#endif	// WITH_EDITOR [UEditorUtils::IsEditorNotPieWorld]

	return Get().LevelMapInternal.Get();
}

// Contains a data of standalone and PIE games, nullptr otherwise
UMyGameInstance* USingletonLibrary::GetMyGameInstance()
{
	const UWorld* World = Get().GetWorld();
	return World ? World->GetGameInstance<UMyGameInstance>() : nullptr;
}

// Contains a data of Bomber Level, nullptr otherwise
AMyGameModeBase* USingletonLibrary::GetMyGameMode()
{
	const UWorld* World = Get().GetWorld();
	return World ? World->GetAuthGameMode<AMyGameModeBase>() : nullptr;
}

// Returns the Bomber Game state, nullptr otherwise.
AMyGameStateBase* USingletonLibrary::GetMyGameState()
{
	const UWorld* World = Get().GetWorld();
	return World ? World->GetGameState<AMyGameStateBase>() : nullptr;
}

// Returns the Bomber Player Controller, nullptr otherwise
AMyPlayerController* USingletonLibrary::GetMyPlayerController(int32 PlayerIndex)
{
	const AMyGameModeBase* MyGameMode = GetMyGameMode();
	AMyPlayerController* MyPC = MyGameMode ? MyGameMode->GetPlayerController(PlayerIndex) : nullptr;
	if (MyPC)
	{
		return MyPC;
	}

	return Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(&Get(), PlayerIndex));
}

// Returns the local Player Controller, nullptr otherwise
AMyPlayerController* USingletonLibrary::GetLocalPlayerController()
{
	static constexpr int32 LocalPlayerIndex = 0;
	return GetMyPlayerController(LocalPlayerIndex);
}

// Returns the Bomber Player State for specified player, nullptr otherwise
AMyPlayerState* USingletonLibrary::GetMyPlayerState(const APawn* Pawn)
{
	return Pawn ? Pawn->GetPlayerState<AMyPlayerState>() : nullptr;
}

// Returns the player state of current controller
AMyPlayerState* USingletonLibrary::GetLocalPlayerState()
{
	const AMyPlayerController* MyPlayerController = GetLocalPlayerController();
	return MyPlayerController ? MyPlayerController->GetPlayerState<AMyPlayerState>() : nullptr;
}

// Returns the Bomber settings
UMyGameUserSettings* USingletonLibrary::GetMyGameUserSettings()
{
	return GEngine ? Cast<UMyGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

// Returns the settings widget
USettingsWidget* USingletonLibrary::GetSettingsWidget()
{
	const AMyHUD* MyHUD = GetMyHUD();
	return MyHUD ? MyHUD->GetSettingsWidget() : nullptr;
}

// Returns the Camera Component used on level
UMyCameraComponent* USingletonLibrary::GetLevelCamera()
{
	return AGeneratedMap::Get().GetCameraComponent();
}

// Returns the HUD actor
AMyHUD* USingletonLibrary::GetMyHUD()
{
	const AMyPlayerController* MyPlayerController = GetLocalPlayerController();
	return MyPlayerController ? MyPlayerController->GetHUD<AMyHUD>() : nullptr;
}

// Returns the Main Menu widget
UMainMenuWidget* USingletonLibrary::GetMainMenuWidget()
{
	const AMyHUD* MyHUD = GetMyHUD();
	return MyHUD ? MyHUD->GetMainMenuWidget() : nullptr;
}

// Returns the In-Game widget
UInGameWidget* USingletonLibrary::GetInGameWidget()
{
	const AMyHUD* MyHUD = GetMyHUD();
	return MyHUD ? MyHUD->GetInGameWidget() : nullptr;
}

// Returns the In-Game Menu widget
UInGameMenuWidget* USingletonLibrary::GetInGameMenuWidget()
{
	const UInGameWidget* InGameWidget = GetInGameWidget();
	return InGameWidget ? InGameWidget->GetInGameMenuWidget() : nullptr;
}

// Returns specified player character, by default returns local player
APlayerCharacter* USingletonLibrary::GetPlayerCharacter(int32 PlayerIndex)
{
	const AMyPlayerController* MyPC = GetMyPlayerController(PlayerIndex);
	return MyPC ? MyPC->GetPawn<APlayerCharacter>() : nullptr;
}

// Returns controlled player character
APlayerCharacter* USingletonLibrary::GetLocalPlayerCharacter()
{
	static constexpr int32 LocalPlayerIndex = 0;
	return GetPlayerCharacter(LocalPlayerIndex);
}

// Returns the Sound Manager
USoundsManager* USingletonLibrary::GetSoundsManager()
{
	return USoundsDataAsset::Get().GetSoundsManager();
}

// Returns the Pool Manager of the game that is used to reuse created objects
UPoolManager* USingletonLibrary::GetPoolManager()
{
	return AGeneratedMap::Get().GetPoolManager();
}

/* ---------------------------------------------------
 *		Editor development
 * --------------------------------------------------- */

#if WITH_EDITOR
// Will notify on any data asset changes
USingletonLibrary::FOnAnyDataAssetChanged USingletonLibrary::GOnAnyDataAssetChanged;
#endif //WITH_EDITOR

// Remove all text renders of the Owner
void USingletonLibrary::ClearDisplayedCells(AActor* Owner)
{
	#if WITH_EDITOR	 // [UEditorUtils::IsEditor]
	if (!UEditorUtilsLibrary::IsEditor()
	    || !IS_VALID(Owner)) // The owner is not valid
	{
		return;
	}

	TArray<UActorComponent*> TextRendersArray;
	Owner->GetComponents(UTextRenderComponent::StaticClass(), TextRendersArray);
	for (int32 i = TextRendersArray.Num() - 1; i >= 0; --i)
	{
		UTextRenderComponent* TextRenderIt = TextRendersArray.IsValidIndex(i) ? Cast<UTextRenderComponent>(TextRendersArray[i]) : nullptr;
		if (!TextRenderIt)
		{
			continue;
		}

		const FName NameIt = *TextRenderIt->Text.ToString();
		static const FName DefaultPlayerName = "Player";
		static const FName DefaultAIName = "AI";
		if (NameIt != DefaultPlayerName
		    && NameIt != DefaultAIName)
		{
			TextRenderIt->DestroyComponent();
		}
	}
#endif	// WITH_EDITOR [UEditorUtils::IsEditor]
}

// Debug visualization by text renders
void USingletonLibrary::AddDebugTextRenders_Implementation(AActor* Owner, const FCells& Cells, const FLinearColor& TextColor, bool& bOutHasCoordinateRenders, TArray<UTextRenderComponent*>& OutTextRenderComponents, float TextHeight, float TextSize, const FString& RenderString, const FVector& CoordinatePosition) const
{
#if WITH_EDITOR	 // [UEditorUtils::IsEditor]
	if (!UEditorUtilsLibrary::IsEditor()
	    || !Cells.Num()      // Null length
	    || !IS_VALID(Owner)) // Owner is not valid
	{
		return;
	}

	bOutHasCoordinateRenders = (CoordinatePosition.IsZero() == false && RenderString.IsEmpty() == false);
	OutTextRenderComponents.SetNum(bOutHasCoordinateRenders ? Cells.Num() * 2 : Cells.Num());
	for (UTextRenderComponent*& TextRenderIt : OutTextRenderComponents)
	{
		TextRenderIt = NewObject<UTextRenderComponent>(Owner);
		TextRenderIt->RegisterComponent();
	}
#endif	// WITH_EDITOR [UEditorUtils::IsEditor]
}

void USingletonLibrary::DisplayCells(AActor* Owner, const FCells& Cells, const FDisplayCellsParams& Params)
{
#if WITH_EDITOR	 // [UEditorUtils::IsEditor]
	if (UEditorUtilsLibrary::IsEditor())
	{
		bool bOutBool = false;
		TArray<UTextRenderComponent*> OutArray;
		Get().AddDebugTextRenders(Owner, Cells, Params.TextColor, bOutBool, OutArray, Params.TextHeight, Params.TextSize, Params.RenderString, Params.CoordinatePosition);
	}
#endif	// WITH_EDITOR [UEditorUtils::IsEditor]
}
