// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyCheatManager.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Controllers/MyAIController.h"
#include "UtilityLibraries/SingletonLibrary.h"
#include "LevelActors/BoxActor.h"
#include "LevelActors/ItemActor.h"
#include "LevelActors/PlayerCharacter.h"
#include "UI/MyHUD.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"

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
	FMapComponents PlayerComponents;
	AGeneratedMap::Get().GetMapComponents(PlayerComponents, TO_FLAG(EActorType::Player));
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
	FCells Cells;
	UCellsUtilsLibrary::GetAllCellsByActors(Cells, TO_FLAG(ActorType));
	AGeneratedMap::Get().DestroyActorsFromMap(Cells);
}

// Destroy characters in specified slots
void UMyCheatManager::DestroyPlayersBySlots(const FString& Slot) const
{
	// Set bitmask
	const int32 Bitmask = GetBitmask(Slot);
	if (!Bitmask)
	{
		return;
	}

	AGeneratedMap& LevelMap = AGeneratedMap::Get();

	// Get all players
	FCells CellsToDestroy;
	FMapComponents MapComponents;
	LevelMap.GetMapComponents(MapComponents, TO_FLAG(EActorType::Player));
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		const APlayerCharacter* PlayerCharacter = MapComponentIt ? Cast<APlayerCharacter>(MapComponentIt->GetOwner()) : nullptr;
		const bool bDestroy = PlayerCharacter && ((1 << PlayerCharacter->GetCharacterID()) & Bitmask) != 0;
		if (bDestroy) // mark to destroy if specified in slot
		{
			CellsToDestroy.Emplace(MapComponentIt->GetCell());
		}
	}

	// Destroy all specified
	LevelMap.DestroyActorsFromMap(CellsToDestroy);
}

// Override the chance to spawn item after box destroying
void UMyCheatManager::SetItemChance(int32 Chance) const
{
	// Get all boxes
	FCells CellsToDestroy;
	FMapComponents MapComponents;
	AGeneratedMap::Get().GetMapComponents(MapComponents, TO_FLAG(EActorType::Box));
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
	if (APlayerCharacter* PlayerCharacter = USingletonLibrary::GetLocalPlayerCharacter())
	{
		static constexpr int32 MinItemsNum = 1;
		const int32 MaxItemsNum = UItemDataAsset::Get().GetMaxAllowedItemsNum();
		NewLevel = FMath::Clamp(NewLevel, MinItemsNum, MaxItemsNum);
		PlayerCharacter->PowerupsInternal.BombN = NewLevel;
		PlayerCharacter->PowerupsInternal.FireN = NewLevel;
		PlayerCharacter->PowerupsInternal.SkateN = NewLevel;
		PlayerCharacter->ApplyPowerups();
	}
}

// Enable or disable the God mode to make a controllable player undestroyable
void UMyCheatManager::SetGodMode(bool bShouldEnable) const
{
	const APlayerCharacter* ControllablePlayer = USingletonLibrary::GetLocalPlayerCharacter();
	if (UMapComponent* MapComponent = UMapComponent::GetMapComponent(ControllablePlayer))
	{
		MapComponent->SetUndestroyable(bShouldEnable);
	}
}

// Set new setting value
void UMyCheatManager::SetSetting(const FString& TagByValue) const
{
	if (TagByValue.IsEmpty())
	{
		return;
	}

	static const FString Delimiter = TEXT("?");
	TArray<FString> SeparatedStrings;
	TagByValue.ParseIntoArray(SeparatedStrings, *Delimiter);

	static constexpr int32 TagIndex = 0;
	FName TagName = NAME_None;
	if (SeparatedStrings.IsValidIndex(TagIndex))
	{
		TagName = *SeparatedStrings[TagIndex];
	}

	if (TagName.IsNone())
	{
		return;
	}

	// Extract value
	static constexpr int32 ValueIndex = 1;
	FString TagValue = TEXT("");
	if (SeparatedStrings.IsValidIndex(ValueIndex))
	{
		TagValue = SeparatedStrings[ValueIndex];
	}

	const AMyHUD* MyHUD = USingletonLibrary::GetMyHUD();
	USettingsWidget* SettingsWidget = MyHUD ? MyHUD->GetSettingsWidget() : nullptr;
	if (SettingsWidget)
	{
		SettingsWidget->SetSettingValue(TagName, TagValue);
		SettingsWidget->SaveSettings();
	}
}
