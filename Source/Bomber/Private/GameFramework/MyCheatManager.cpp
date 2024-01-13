// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyCheatManager.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Components/MyCameraComponent.h"
#include "DataAssets/ItemDataAsset.h"
#include "LevelActors/BoxActor.h"
#include "LevelActors/PlayerCharacter.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyCheatManager)

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
	GeneratedMap.GetMapComponents(MapComponents, TO_FLAG(EAT::Player));
	for (const UMapComponent* MapComponentIt : MapComponents)
	{
		const APlayerCharacter* PlayerCharacter = MapComponentIt ? MapComponentIt->GetOwner<APlayerCharacter>() : nullptr;
		const bool bDestroy = PlayerCharacter && (1 << PlayerCharacter->GetCharacterID() & Bitmask) != 0;
		if (bDestroy) // mark to destroy if specified in slot
		{
			CellsToDestroy.Emplace(MapComponentIt->GetCell());
		}
	}

	// Destroy all specified
	GeneratedMap.DestroyLevelActorsOnCells(CellsToDestroy);
}

// Override the chance to spawn item after box destroying
void UMyCheatManager::SetItemChance(int32 Chance)
{
	// Get all boxes
	FMapComponents MapComponents;
	AGeneratedMap::Get().GetMapComponents(MapComponents, TO_FLAG(EAT::Box));
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

// Override the level of each powerup for a controlled player
void UMyCheatManager::SetPowerups(int32 NewLevel)
{
	if (APlayerCharacter* PlayerCharacter = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter())
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
void UMyCheatManager::SetGodMode(bool bShouldEnable)
{
	const APlayerCharacter* ControllablePlayer = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	if (UMapComponent* MapComponent = UMapComponent::GetMapComponent(ControllablePlayer))
	{
		MapComponent->SetUndestroyable(bShouldEnable);
	}
}

// Shows coordinates of all level actors by specified types
void UMyCheatManager::DisplayCells(const FString& ActorTypesString)
{
	// Set on the level to visualize new level actors
	const int32 ActorTypesBitmask = GetBitmaskFromActorTypesString(ActorTypesString);
	AGeneratedMap::Get().DisplayCellsActorTypes = ActorTypesBitmask;

	// Update existed level actors
	FMapComponents MapComponents;
	AGeneratedMap::Get().GetMapComponents(MapComponents, TO_FLAG(EAT::All));
	for (UMapComponent* MapComponentIt : MapComponents)
	{
		// Clear previous cell renders for all level actors in game
		UCellsUtilsLibrary::ClearDisplayedCells(MapComponentIt);

		// Show new cell renders for specified level actors
		MapComponentIt->TryDisplayOwnedCell();
	}
}

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

// Tweak the custom additive angle to affect the fit distance calculation from camera to the level
void UMyCheatManager::FitViewAdditiveAngle(float InFitViewAdditiveAngle)
{
	if (UMyCameraComponent* LevelCamera = UMyBlueprintFunctionLibrary::GetLevelCamera())
	{
		FCameraDistanceParams DistanceParams = LevelCamera->GetCameraDistanceParams();
		DistanceParams.FitViewAdditiveAngle = InFitViewAdditiveAngle;
		LevelCamera->SetCameraDistanceParams(MoveTemp(DistanceParams));
	}
}

// Tweak the minimal distance in UU from camera to the level
void UMyCheatManager::MinDistance(float InMinDistance)
{
	if (UMyCameraComponent* LevelCamera = UMyBlueprintFunctionLibrary::GetLevelCamera())
	{
		FCameraDistanceParams DistanceParams = LevelCamera->GetCameraDistanceParams();
		DistanceParams.MinDistance = InMinDistance;
		LevelCamera->SetCameraDistanceParams(MoveTemp(DistanceParams));
	}
}
