// Copyright 2021 Yevhenii Selivanov.

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
#include "UI/InputControlsWidget.h"
//---
#include "Engine.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
//---
#if WITH_EDITOR
#include "Editor.h"	 // GEditor
#endif

// Binds to update movements of each AI controller.
USingletonLibrary::FUpdateAI USingletonLibrary::GOnAIUpdatedDelegate;

/* ---------------------------------------------------
 *		Editor development
 * --------------------------------------------------- */

#if WITH_EDITOR
// Will notify on any data asset changes
USingletonLibrary::FOnAnyDataAssetChanged USingletonLibrary::GOnAnyDataAssetChanged;
#endif //WITH_EDITOR

// Returns a world of stored level map
UWorld* USingletonLibrary::GetWorld() const
{
	const AGeneratedMap* LevelMap = GetLevelMap();
	return LevelMap ? LevelMap->GetWorld() : nullptr;
}

bool USingletonLibrary::IsEditor()
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	return GIsEditor && GWorld && GWorld->IsEditorWorld();
#endif
	return false;
}

// Checks, that this actor placed in the editor world and the game is not started yet
bool USingletonLibrary::IsEditorNotPieWorld()
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	return IsEditor() && GEditor && !GEditor->IsPlaySessionInProgress();
#endif	// [IsEditorNotPieWorld]
	return false;
}

// Remove all text renders of the Owner
void USingletonLibrary::ClearOwnerTextRenders(AActor* Owner)
{
#if WITH_EDITOR	 // [IsEditor]
	if (!IsEditor()
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
#endif	// WITH_EDITOR [IsEditor]
}

// Debug visualization by text renders
void USingletonLibrary::AddDebugTextRenders_Implementation(AActor* Owner, const TSet<FCell>& Cells, const FLinearColor& TextColor, bool& bOutHasCoordinateRenders, TArray<UTextRenderComponent*>& OutTextRenderComponents, float TextHeight, float TextSize, const FString& RenderString, const FVector& CoordinatePosition) const
{
#if WITH_EDITOR	 // [IsEditor]
	if (!IsEditor()
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
#endif	// WITH_EDITOR [IsEditor]
}

void USingletonLibrary::AddDebugTextRenders(
	AActor* Owner,
	const FCells& Cells,
	const FLinearColor& TextColor,
	float TextHeight/* = 261.f*/,
	float TextSize/* = 124.f*/,
	const FString& RenderString/* = TEXT("")*/,
	const FVector& CoordinatePosition/* = FVector::ZeroVector*/)
{
	bool bOutBool = false;
	TArray<UTextRenderComponent*> OutArray;
	Get().AddDebugTextRenders(Owner, Cells, TextColor, bOutBool, OutArray, TextHeight, TextSize, RenderString, CoordinatePosition);
}

/* ---------------------------------------------------
 *		Static library functions
 * --------------------------------------------------- */

//  Returns the singleton, nullptr otherwise
USingletonLibrary* USingletonLibrary::GetSingleton()
{
	USingletonLibrary* Singleton = GEngine ? Cast<USingletonLibrary>(GEngine->GameSingleton) : nullptr;
	checkf(Singleton, TEXT("The Singleton is null"));
	return Singleton;
}

// The Level Map getter, nullptr otherwise
AGeneratedMap* USingletonLibrary::GetLevelMap()
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (IsEditorNotPieWorld()
	    && !Get().LevelMapInternal.IsValid())
	{
		SetLevelMap(nullptr); // Find the Level Map
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	return Get().LevelMapInternal.Get();
}

// Returns true if game was started
bool USingletonLibrary::HasWorldBegunPlay()
{
#if WITH_EDITOR	// [IsEditor]
	if (IsEditor())
	{
		return GEditor && GEditor->IsPlaySessionInProgress();
	}
#endif	// [IsEditor]

	const UWorld* World = Get().GetWorld();
	return World && World->HasBegunPlay();
}

// The Level Map setter. If the specified Level Map is not valid or is transient, find and set another one
void USingletonLibrary::SetLevelMap(AGeneratedMap* LevelMap)
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (IsEditorNotPieWorld() // IsEditorNotPieWorld only
	    && LevelMap == nullptr)
	{
		const UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (EditorWorld)
		{
			TArray<AActor*> LevelMapsArray;
			UGameplayStatics::GetAllActorsOfClass(EditorWorld, AGeneratedMap::StaticClass(), LevelMapsArray);
			if (LevelMapsArray.IsValidIndex(0))
			{
				LevelMap = Cast<AGeneratedMap>(LevelMapsArray[0]);
			}
		}
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	if (IS_VALID(LevelMap))
	{
		GetSingleton()->LevelMapInternal = LevelMap;
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

// Contains a data of standalone and PIE games, nullptr otherwise
UMyGameInstance* USingletonLibrary::GetMyGameInstance()
{
	return Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(Get().GetWorld()));
}

// Contains a data of Bomber Level, nullptr otherwise
AMyGameModeBase* USingletonLibrary::GetMyGameMode()
{
	return Cast<AMyGameModeBase>(UGameplayStatics::GetGameMode(Get().GetWorld()));
}

// Returns the Bomber Game state, nullptr otherwise.
AMyGameStateBase* USingletonLibrary::GetMyGameState()
{
	return Cast<AMyGameStateBase>(UGameplayStatics::GetGameState(Get().GetWorld()));
}

// Returns the Bomber Player Controller, nullptr otherwise
AMyPlayerController* USingletonLibrary::GetMyPlayerController()
{
	return Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(Get().GetWorld(), 0));
}

// Returns the Bomber Player State for specified player, nullptr otherwise
AMyPlayerState* USingletonLibrary::GetMyPlayerState(const APawn* Pawn)
{
	return Pawn ? Pawn->GetPlayerState<AMyPlayerState>() : nullptr;
}

// Returns the player state of current controller
AMyPlayerState* USingletonLibrary::GetCurrentPlayerState()
{
	const AMyPlayerController* MyPlayerController = GetMyPlayerController();
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
	const AMyPlayerController* MyPlayerController = GetMyPlayerController();
	return MyPlayerController ? MyPlayerController->GetHUD<AMyHUD>() : nullptr;
}

// Returns the Main Menu widget
UMainMenuWidget* USingletonLibrary::GetMainMenuWidget()
{
	const AMyHUD* MyHUD = GetMyHUD();
	return MyHUD ? MyHUD->GetMainMenuWidget() : nullptr;
}

// Returns the In-Game Menu widget
UInGameWidget* USingletonLibrary::GetInGameWidget()
{
	const AMyHUD* MyHUD = GetMyHUD();
	return MyHUD ? MyHUD->GetInGameWidget() : nullptr;
}

// Returns controlled player character
APlayerCharacter* USingletonLibrary::GetControllablePlayer()
{
	return Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(Get().GetWorld(), 0));
}

// Returns the Sound Manager
USoundsManager* USingletonLibrary::GetSoundsManager()
{
	return USoundsDataAsset::Get().GetSoundsManager();
}

/* ---------------------------------------------------
 *		Structs functions
 * --------------------------------------------------- */

// Find the average of an array of vectors
FCell USingletonLibrary::GetCellArrayAverage(const FCells& Cells)
{
	FVector Sum = FVector::ZeroVector;
	FVector Average = FVector::ZeroVector;
	const float CellsNum = static_cast<float>(Cells.Num());
	if (CellsNum > 0.f)
	{
		for (const FCell& CellIt : Cells)
		{
			Sum += CellIt.Location;
		}

		Average = Sum / CellsNum;
	}

	return FCell(Average);
}

/* ---------------------------------------------------
*		Data assets
* --------------------------------------------------- */

// Iterate ActorsDataAssets array and returns the found Level Actor class by specified data asset
ULevelActorDataAsset* USingletonLibrary::GetDataAssetByActorClass(const TSubclassOf<AActor>& ActorClass)
{
	const TArray<TObjectPtr<ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (const TObjectPtr<ULevelActorDataAsset>& DataAssetIt : ActorsDataAssets)
	{
		if (DataAssetIt
		    && DataAssetIt->GetActorClass()->IsChildOf(ActorClass))
		{
			return DataAssetIt;
		}
	}
	return nullptr;
}

// Iterate ActorsDataAssets array and returns the found Data Assets of level actors by specified types.
void USingletonLibrary::GetDataAssetsByActorTypes(TArray<ULevelActorDataAsset*>& OutDataAssets, int32 ActorsTypesBitmask)
{
	const TArray<TObjectPtr<ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (const TObjectPtr<ULevelActorDataAsset>& DataAssetIt : ActorsDataAssets)
	{
		if (DataAssetIt
		    && EnumHasAnyFlags(DataAssetIt->GetActorType(), TO_ENUM(EActorType, ActorsTypesBitmask)))
		{
			OutDataAssets.Emplace(DataAssetIt);
		}
	}
}

// Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type
ULevelActorDataAsset* USingletonLibrary::GetDataAssetByActorType(EActorType ActorType)
{
	TArray<ULevelActorDataAsset*> FoundDataAssets;
	GetDataAssetsByActorTypes(FoundDataAssets, TO_FLAG(ActorType));
	return FoundDataAssets.IsValidIndex(0) ? FoundDataAssets[0] : nullptr;
}

// Iterate ActorsDataAssets array and returns the found actor class by specified actor type
TSubclassOf<AActor> USingletonLibrary::GetActorClassByType(EActorType ActorType)
{
	const ULevelActorDataAsset* FoundDataAsset = GetDataAssetByActorType(ActorType);
	return FoundDataAsset ? FoundDataAsset->GetActorClass() : nullptr;
}
