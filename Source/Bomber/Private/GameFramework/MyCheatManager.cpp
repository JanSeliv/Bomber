// Copyright 2020 Yevhenii Selivanov

#include "GameFramework/MyCheatManager.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Controllers/MyAIController.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/BoxActor.h"
#include "LevelActors/PlayerCharacter.h"

// Called when CheatManager is created to allow any needed initialization
void UMyCheatManager::InitCheatManager()
{
	Super::InitCheatManager();
}

// Returns bitmask by string
int32 UMyCheatManager::GetBitmask(const FString& String) const
{
	int32 Bitmask = 0, Index = 0;
	for (int32 It = 0; It < String.Len(); ++It)
	{
		FString Char = String.Mid(It, 1);
		if (Char.IsNumeric())
		{
			const int32 Bit = !FCString::Atoi(*Char) ? 0 : 1;
			Bitmask |= Bit << Index++;
		}
	}

	return Bitmask;
}

// Enable or disable all bots
void UMyCheatManager::SetAI(bool bShouldEnable) const
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap)
	{
		return;
	}

	TSet<UMapComponent*> PlayerComponents;
	LevelMap->GetMapComponents(PlayerComponents, TO_FLAG(EActorType::Player));
	for (const UActorComponent* MapComponentIt : PlayerComponents)
	{
		const APawn* Pawn = MapComponentIt ? Cast<APawn>(MapComponentIt->GetOwner()) : nullptr;
		const auto MyAIController = Pawn->GetController<AMyAIController>();
		if (MyAIController)
		{
			MyAIController->SetAI(bShouldEnable);
		}
	}
}

// Destroy all specified level actors on the map
void UMyCheatManager::DestroyAllByType(EActorType ActorType) const
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap)
	{
		return;
	}

	FCells Cells;
	LevelMap->IntersectCellsByTypes(Cells, TO_FLAG(ActorType), true);
	LevelMap->DestroyActorsFromMap(Cells);
}

// Destroy characters in specified slots
void UMyCheatManager::DestroyPlayersBySlots(const FString& Slot) const
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap)
	{
		return;
	}

	// Display debug information
	DisplayDebug();

	// Set bitmask
	const int32 Bitmask = GetBitmask(Slot);
	if (!Bitmask)
	{
		return;
	}

	// Get all players
	FCells CellsToDestroy;
	TSet<UMapComponent*> MapComponents;
	LevelMap->GetMapComponents(MapComponents, TO_FLAG(EActorType::Player));
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		const APlayerCharacter* PlayerCharacter = MapComponentIt ? Cast<APlayerCharacter>(MapComponentIt->GetOwner()) : nullptr;
		const bool bDestroy = PlayerCharacter && ((1 << PlayerCharacter->GetCharacterID()) & Bitmask) != 0;
		if (bDestroy) // mark to destroy if specified in slot
		{
			CellsToDestroy.Emplace(MapComponentIt->Cell);
		}
	}

	// Destroy all specified
	LevelMap->DestroyActorsFromMap(CellsToDestroy);
}

//
void UMyCheatManager::SetItemChance(int32 Chance) const
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap)
	{
		return;
	}

	// Get all boxes
	FCells CellsToDestroy;
	TSet<UMapComponent*> MapComponents;
	LevelMap->GetMapComponents(MapComponents, TO_FLAG(EActorType::Box));
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		ABoxActor* BoxActor = MapComponentIt ? Cast<ABoxActor>(MapComponentIt->GetOwner()) : nullptr;
		if (BoxActor)
		{
			// Override new chance
			BoxActor->SpawnItemChanceInternal = Chance;
		}
	}
}

// Override the level of each powerup for a controlled player
void UMyCheatManager::SetPowerups(int32 NewLevel) const
{
	if (const auto PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		NewLevel = FMath::Clamp(NewLevel, 1, 9);
		PlayerCharacter->PowerupsInternal.BombN = NewLevel;
		PlayerCharacter->PowerupsInternal.FireN = NewLevel;
		PlayerCharacter->PowerupsInternal.SkateN = NewLevel;
	}
}

// Display debug information
void UMyCheatManager::DisplayDebug() const
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->ConsoleCommand("DisplayAll MyGameStateBase CurrentGameStateInternal");
		PC->ConsoleCommand("DisplayAll MyPlayerState EndGameStateInternal");
		PC->ConsoleCommand("DisplayAll GeneratedMap PlayersNumInternal");
	}
}
