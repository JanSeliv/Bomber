// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrCheatManager.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "Bomber.h"
#include "Components/BmrCameraComponent.h"
#include "Components/BmrMapComponent.h"
#include "Controllers/BmrAIController.h"
#include "Controllers/BmrDebugCameraController.h"
#include "Controllers/BmrPlayerController.h"
#include "DataAssets/BmrDataAssetsContainer.h"
#include "DataAssets/BmrGeneratedMapDataAsset.h"
#include "DataAssets/BmrPlayerDataAsset.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/PlayerState.h"
#include "Structures/BmrGameplayTags.h"
#include "Structures/BmrPowerupTag.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCheatManager)

// Default constructor
UBmrCheatManager::UBmrCheatManager()
{
	DebugCameraControllerClass = ABmrDebugCameraController::StaticClass();
}

/*********************************************************************************************
 * Utils
 ********************************************************************************************* */

// Returns bitmask from reverse bitmask in string
int32 UBmrCheatManager::GetBitmaskFromReverseString(const FString& ReverseBitmaskStr)
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
int32 UBmrCheatManager::GetBitmaskFromActorTypesString(const FString& ActorTypesBitmaskStr)
{
	if (ActorTypesBitmaskStr.IsEmpty())
	{
		return 0;
	}

	static const FString Delimiter = TEXT(" ");
	TArray<FString> ActorTypesStrings;
	ActorTypesBitmaskStr.ParseIntoArray(ActorTypesStrings, *Delimiter);

	const static FString ActorTypeEnumPathName = TEXT("/Script/Bomber.EBmrActorType");
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
void UBmrCheatManager::DestroyAllByType(EBmrActorType ActorType)
{
	const FBmrCells Cells = UBmrCellUtilsLibrary::GetAllCellsWithActors(TO_FLAG(ActorType));
	ABmrGeneratedMap::Get().DestroyLevelActorsOnCells(Cells);
}

// Destroy characters in specified slots
void UBmrCheatManager::DestroyPlayersBySlots(const FString& Slot)
{
	// Set bitmask
	const int32 Bitmask = GetBitmaskFromReverseString(Slot);
	if (!Bitmask)
	{
		return;
	}

	// Get all players
	FBmrCells CellsToDestroy;
	FMapComponents MapComponents;
	UBmrActorUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::Player));
	for (const UBmrMapComponent* MapComponentIt : MapComponents)
	{
		const ABmrPawn* Pawn = MapComponentIt ? MapComponentIt->GetOwner<ABmrPawn>() : nullptr;
		UAbilitySystemComponent* ASC = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
		if (!ASC)
		{
			continue;
		}

		const int32 PlayerIndex = Pawn->GetPlayerId();
		const bool bIsMatchInSlot = (1 << PlayerIndex & Bitmask) != 0;
		if (bIsMatchInSlot)
		{
			FGameplayEventData EventData;
			EventData.EventTag = BmrGameplayTags::Event::Player_Death;
			EventData.Instigator = UBmrBlueprintFunctionLibrary::GetLocalPawn();
			EventData.Target = Pawn;
			UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);
		}
	}
}

/*********************************************************************************************
 * Box
 ********************************************************************************************* */

// Override the percentage of powerups spawn from boxes
TAutoConsoleVariable<int32> UBmrCheatManager::CVarPowerupsChance(
    TEXT("Bomber.Box.SetPowerupsChance"),
    0.f,
    TEXT("100 - is maximum, 0 - is disabled (default chance will be used)"),
    ECVF_Cheat);

/*********************************************************************************************
 * Player
 ********************************************************************************************* */

// Is overridden to apply damage immunity effect for proper integration with Ability System
void UBmrCheatManager::God()
{
	// Super is not called intentionally to implement custom god mode through GAS

	const ABmrPawn* Pawn = UBmrBlueprintFunctionLibrary::GetLocalPawn();
	UAbilitySystemComponent* ASC = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
	const TSubclassOf<UGameplayEffect> BlockIncomingDamageEffect = UBmrPlayerDataAsset::Get().GetBlockIncomingDamageEffect();
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(BlockIncomingDamageEffect, TEXT("ASSERT: [%i] %hs:\n'BlockIncomingDamageEffect' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const FGameplayTag BlockIncomingDamageTag = BmrGameplayTags::GameplayEffect::Block::IncomingDamage;
	if (!ASC->HasMatchingGameplayTag(BlockIncomingDamageTag))
	{
		// Effect is not applied yet, so apply it
		ASC->ApplyGameplayEffectToSelf(BlockIncomingDamageEffect.GetDefaultObject(), /*Level*/ 1.f, ASC->MakeEffectContext());
	}
	else
	{
		// Effect is already applied (in god mode), so remove it
		ASC->RemoveActiveEffectsWithGrantedTags(BlockIncomingDamageTag.GetSingleTagContainer());
	}
}

// Forcing locally controlled player to destroy itself immediately, resulting in loss
void UBmrCheatManager::Suicide()
{
	const ABmrPawn* Pawn = UBmrBlueprintFunctionLibrary::GetLocalPawn();
	const int32 PlayerId = Pawn ? Pawn->GetPlayerId() : INDEX_NONE;
	if (PlayerId < 0)
	{
		return;
	}

	FString SlotString = FString::ChrN(PlayerId, TEXT('0'));
	SlotString.AppendChar(TEXT('1'));
	DestroyPlayersBySlots(SlotString);
}

// Override the level of each powerup for a controlled player
void UBmrCheatManager::SetPlayerPowerups(int32 NewLevel)
{
	const ABmrPawn* Pawn = UBmrBlueprintFunctionLibrary::GetLocalPawn();
	UAbilitySystemComponent* ASC = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	for (const FGameplayTag& PowerupTagIt : FBmrPowerupTag::GetAll())
	{
		const FGameplayAttribute PowerupAttribute = UBmrPowerupsAttributeSet::Conv_TagToBaseAttribute(PowerupTagIt);
		if (PowerupAttribute.IsValid())
		{
			ASC->ApplyModToAttributeUnsafe(PowerupAttribute, EGameplayModOp::Override, NewLevel);
		}
	}
}

// Enable or disable the Auto Copilot mode to make a controllable player to play automatically
void UBmrCheatManager::SetAutoCopilot()
{
	ABmrPawn* LocalPlayer = UBmrBlueprintFunctionLibrary::GetLocalPawn();
	APlayerState* PlayerState = LocalPlayer ? LocalPlayer->GetPlayerState() : nullptr;
	if (!PlayerState)
	{
		return;
	}

	// Toggle the Copilot mode
	PlayerState->SetIsABot(!PlayerState->IsABot());
	LocalPlayer->TryPossessController(EBmrPlayerType::Any);
}

/*********************************************************************************************
 * AI
 ********************************************************************************************* */

// Enable or disable all bots
TAutoConsoleVariable<bool> UBmrCheatManager::CVarAISetEnabled(
    TEXT("Bomber.AI.SetEnabled"),
    true,
    TEXT("Enable or disable all bots: 1 (Enable) OR 0 (Disable)"),
    ECVF_Cheat);

// Override the level of each powerup for bots
void UBmrCheatManager::SetAIPowerups(int32 NewLevel)
{
	// Get all players
	FMapComponents MapComponents;
	UBmrActorUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::Player));

	// Override the level of each powerup for bots
	for (const UBmrMapComponent* MapComponentIt : MapComponents)
	{
		const ABmrPawn* Pawn = MapComponentIt ? MapComponentIt->GetOwner<ABmrPawn>() : nullptr;
		UAbilitySystemComponent* ASC = Pawn && Pawn->IsBotControlled() ? Pawn->GetAbilitySystemComponent() : nullptr;
		if (ASC)
		{
			ASC->ApplyModToAttributeUnsafe(UBmrPowerupsAttributeSet::GetPowerup_SkateAttribute(), EGameplayModOp::Override, NewLevel);
			ASC->ApplyModToAttributeUnsafe(UBmrPowerupsAttributeSet::GetPowerup_FireAttribute(), EGameplayModOp::Override, NewLevel);
			ASC->ApplyModToAttributeUnsafe(UBmrPowerupsAttributeSet::GetPowerup_BombsAvailableAttribute(), EGameplayModOp::Override, NewLevel);
		}
	}
}

// If called, all bots will change own skin to look like players
void UBmrCheatManager::ApplyPlayersSkinOnAI()
{
	// Get all players
	FMapComponents MapComponents;
	UBmrActorUtilsLibrary::GetLevelActors(MapComponents, TO_FLAG(EAT::Player));

	// Apply players skin on AI
	for (const UBmrMapComponent* MapComponentIt : MapComponents)
	{
		ABmrPawn* Character = MapComponentIt ? MapComponentIt->GetOwner<ABmrPawn>() : nullptr;
		if (Character
		    && Character->IsBotControlled())
		{
			constexpr bool bForcePlayerSkin = true;
			Character->SetDefaultPlayerMeshData(bForcePlayerSkin);
		}
	}
}

// Spawns additional bots at the center of the level
void UBmrCheatManager::AddBot()
{
	const FIntPoint CenterPosition = UBmrCellUtilsLibrary::GetCenterCellPositionOnLevel();
	const int32 LastRowIndex = UBmrPlayerDataAsset::Get().GetRowsNum() - 1;
	SpawnActorByType(EBmrActorType::Player, CenterPosition.X, CenterPosition.Y, LastRowIndex);
}

/*********************************************************************************************
 * Debug
 ********************************************************************************************* */

// Shows coordinates of level actors of specified types
TAutoConsoleVariable<FString> UBmrCheatManager::CVarDisplayCells(
    TEXT("Bomber.Debug.DisplayCells"),
    TEXT(""),
    TEXT("Shows coordinates of level actors of specified types (requires regeneration), e.g: Bomber.Debug.DisplayCells Bomb Player - show bombs and players"),
    ECVF_Cheat);

/*********************************************************************************************
 * Level
 ********************************************************************************************* */

// Sets the size for generated map, it will automatically regenerate the level for given size
void UBmrCheatManager::SetLevelSize(const FString& LevelSize)
{
	static const FString Delimiter = TEXT("x");
	FString Width = TEXT("");
	FString Height = TEXT("");

	if (!LevelSize.Split(Delimiter, &Width, &Height, ESearchCase::IgnoreCase))
	{
		return;
	}

	// Update the level size
	const FIntPoint NewLevelSize(FCString::Atoi(*Width), FCString::Atoi(*Height));
	ABmrGeneratedMap::Get().SetLevelSize(NewLevelSize);
}

// Spawns an actor by type on the level
void UBmrCheatManager::SpawnActorByType(EBmrActorType ActorType, int32 ColumnX, int32 RowY, int32 RowIndex)
{
	ABmrGeneratedMap& GeneratedMap = ABmrGeneratedMap::Get();
	const FBmrCell Cell = UBmrCellUtilsLibrary::GetCellByPositionOnLevel(ColumnX, RowY);

	// Destroy existed actor on the cell
	GeneratedMap.DestroyLevelActorsOnCells({Cell});

	// Spawn new actor on the cell
	const UBmrLevelActorDataAsset* DataAsset = UBmrDataAssetsContainer::Get().GetDataAssetByActorType(ActorType);
	checkf(DataAsset, TEXT("ERROR: [%i] %hs:\n'DataAsset' is null!"), __LINE__, __FUNCTION__);
	const UBmrLevelActorRow* Row = DataAsset->GetRowByIndex(RowIndex);
	GeneratedMap.SpawnActorWithMesh(ActorType, Cell, {Row});
}

// Overrides the percentage of walls spawn during the level generation
void UBmrCheatManager::SetWallsChance(int32 WallsChance)
{
	ABmrGeneratedMap& GeneratedMap = ABmrGeneratedMap::Get();
	FBmrGeneratedMapSettings CurrentSettings = GeneratedMap.GetGenerationSetting();

	// Use default from data asset if -1, otherwise use provided value
	constexpr int32 MaxChance = 100;
	WallsChance = FMath::Min(WallsChance, MaxChance);
	CurrentSettings.WallsChance = WallsChance >= 0 ? WallsChance : UBmrGeneratedMapDataAsset::Get().GetGenerationSettings().WallsChance;
	GeneratedMap.SetOverriddenGenerationSettings(/*bEnableOverride*/ true, CurrentSettings);

	GeneratedMap.GenerateLevelActors();
}

// Overrides the percentage of boxes spawn during the level generation
void UBmrCheatManager::SetBoxesChance(int32 BoxesChance)
{
	ABmrGeneratedMap& GeneratedMap = ABmrGeneratedMap::Get();
	FBmrGeneratedMapSettings CurrentSettings = GeneratedMap.GetGenerationSetting();

	// Use default from data asset if -1, otherwise use provided value
	constexpr int32 MaxChance = 100;
	BoxesChance = FMath::Min(BoxesChance, MaxChance);
	CurrentSettings.BoxesChance = BoxesChance >= 0 ? BoxesChance : UBmrGeneratedMapDataAsset::Get().GetGenerationSettings().BoxesChance;
	GeneratedMap.SetOverriddenGenerationSettings(/*bEnableOverride*/ true, CurrentSettings);

	GeneratedMap.GenerateLevelActors();
}

/*********************************************************************************************
 * Camera
 ********************************************************************************************* */

// Is overridden to let internal systems know that camera manager is enabled
void UBmrCheatManager::EnableDebugCamera()
{
	ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	if (ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPC' is null!"), __LINE__, __FUNCTION__))
	{
		// Enable the Debug Camera as early as possible, so during Super call it will be already enabled
		MyPC->bIsDebugCameraEnabled = true;
	}

	Super::EnableDebugCamera();
}

// Is overridden to let internal systems know that camera manager is disabled
void UBmrCheatManager::DisableDebugCamera()
{
	Super::DisableDebugCamera();

	ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	if (ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPC' is null!"), __LINE__, __FUNCTION__))
	{
		// Disable debug camera as late as possible, so during Super call it will be still enabled
		MyPC->bIsDebugCameraEnabled = false;
		MyPC->ApplyAllInputContexts();
	}
}

// Tweak the custom additive angle to affect the fit distance calculation from camera to the level
void UBmrCheatManager::FitViewAdditiveAngle(float InFitViewAdditiveAngle)
{
	if (UBmrCameraComponent* LevelCamera = UBmrBlueprintFunctionLibrary::GetLevelCamera())
	{
		FCameraDistanceParams DistanceParams = LevelCamera->GetCameraDistanceParams();
		DistanceParams.FitViewAdditiveAngle = InFitViewAdditiveAngle;
		LevelCamera->SetCameraDistanceParams(DistanceParams);
	}
}

// Tweak the minimal distance in UU from camera to the level
void UBmrCheatManager::MinDistance(float InMinDistance)
{
	if (UBmrCameraComponent* LevelCamera = UBmrBlueprintFunctionLibrary::GetLevelCamera())
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
void UBmrCheatManager::SetUIHideAllWidgets()
{
	UBmrWidgetsSubsystem* WidgetsSubsystem = UBmrWidgetsSubsystem::GetWidgetsSubsystem();
	if (ensureMsgf(WidgetsSubsystem, TEXT("ASSERT: [%i] %hs:\n'WidgetsSubsystem' is not valid!"), __LINE__, __FUNCTION__))
	{
		const bool bMakeVisibleInversed = WidgetsSubsystem->AreAllWidgetsHidden();
		WidgetsSubsystem->SetAllWidgetsVisibility(bMakeVisibleInversed);
	}
}
