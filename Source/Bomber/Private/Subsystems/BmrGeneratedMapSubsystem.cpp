// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrGeneratedMapSubsystem.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "DalRegistrySubsystem.h"
#include "DalSubsystem.h"
#include "DataAssets/BmrGeneratedMapDataAsset.h"
#include "DataRegistries/BmrLevelActorRow.h"
#include "DataRegistries/BmrPlayerPropRow.h"
#include "DataRegistries/BmrPlayerSkinRow.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif

// UE
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "UObject/UObjectThreadContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGeneratedMapSubsystem)

// Returns the Generated Map Subsystem, is checked and will crash if can't be obtained
UBmrGeneratedMapSubsystem& UBmrGeneratedMapSubsystem::Get(const UObject* WorldContextObject /* = nullptr*/)
{
	UBmrGeneratedMapSubsystem* GeneratedMapSubsystem = GetGeneratedMapSubsystem(WorldContextObject);
	checkf(GeneratedMapSubsystem, TEXT("%s: 'GeneratedMapSubsystem' is null"), *FString(__FUNCTION__));
	return *GeneratedMapSubsystem;
}

// Returns the pointer to the Generated Map Subsystem
UBmrGeneratedMapSubsystem* UBmrGeneratedMapSubsystem::GetGeneratedMapSubsystem(const UObject* WorldContextObject /* = nullptr*/)
{
	const UWorld* FoundWorld = UUtilsLibrary::GetPlayWorld(WorldContextObject);
	return FoundWorld ? FoundWorld->GetSubsystem<UBmrGeneratedMapSubsystem>() : nullptr;
}

/*********************************************************************************************
 * Readiness
 ********************************************************************************************* */

// Returns true the Generated Map actor is ready to generate actors
bool UBmrGeneratedMapSubsystem::IsGeneratedMapReady() const
{
	return GeneratedMap && bAreDataAssetsLoaded && UDalRegistrySubsystem::Get().IsLoaded(this);
}

// Broadcasts GeneratedMap_Ready if both the Generated Map actor and dependent actors' data assets are available
void UBmrGeneratedMapSubsystem::TryBroadcastGeneratedMapReady()
{
	if (!IsGeneratedMapReady())
	{
		return;
	}

	GeneratedMap->OnGeneratedMapReady();

	FGameplayEventData Payload;
	Payload.EventTag = BmrGameplayTags::Event::GeneratedMap_Ready;
	Payload.Instigator = GeneratedMap;
	UGlobalMessageSubsystem::BroadcastGlobalMessage(Payload);
}

/*********************************************************************************************
 * Generated Map
 ********************************************************************************************* */

// The Generated Map getter, nullptr otherwise
ABmrGeneratedMap* UBmrGeneratedMapSubsystem::GetGeneratedMap(bool bWarnIfNull /* = true*/) const
{
#if WITH_EDITOR
	if (bWarnIfNull)
	{
		ensureMsgf(FEditorUtilsLibrary::IsCooking() || GeneratedMap, TEXT("%s: [Editor] 'GeneratedMap' is not valid"), *FString(__FUNCTION__));
	}
#endif // WITH_EDITOR
	return GeneratedMap;
}

// The Generated Map setter
void UBmrGeneratedMapSubsystem::SetGeneratedMap(ABmrGeneratedMap* InGeneratedMap)
{
	if (!ensureMsgf(InGeneratedMap, TEXT("ASSERT: [%i] %hs:\n'InGeneratedMap' is not valid!"), __LINE__, __FUNCTION__)
	    || GeneratedMap == InGeneratedMap)
	{
		return;
	}

	GeneratedMap = InGeneratedMap;

	TryBroadcastGeneratedMapReady();
}

/*********************************************************************************************
 * Overrides and Events
 ********************************************************************************************* */

// Is called on subsystem creation, used for listening the readiness of the Generated Map and its data assets
void UBmrGeneratedMapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UDalRegistrySubsystem& DalRegistry = UDalRegistrySubsystem::Get();
	DalRegistry.BindAndLoadFamily<FBmrLevelActorRow>(this, &ThisClass::OnLevelActorRowsChanged);
	DalRegistry.Add<FBmrPlayerSkinRow>(this);
	DalRegistry.Add<FBmrPlayerPropRow>(this);

	// Listen for dependent data assets to be loaded
	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateWeakLambda(this, [this]
	{
		TArray<TSubclassOf<UDalPrimaryDataAsset>> DataAssetClasses;
		UBmrBlueprintFunctionLibrary::GetDataAssetsByActorTypes(/*out*/ DataAssetClasses, TO_FLAG(EBmrActorType::All));
		ensureMsgf(!DataAssetClasses.IsEmpty(), TEXT("ASSERT: [%i] %hs:\nNo level actor data asset classes found!"), __LINE__, __FUNCTION__);
		DataAssetClasses.Add(UBmrGeneratedMapDataAsset::StaticClass());
		UDalSubsystem::Get().ListenForDataAssets(DataAssetClasses, [World = TWeakObjectPtr(GetWorld())]
		{
			if (UBmrGeneratedMapSubsystem* This = GetGeneratedMapSubsystem(World.Get()))
			{
				This->bAreDataAssetsLoaded = true;
				UDalRegistrySubsystem::Get().TryLoad(This);
			}
		});
	}));
}

// Unsubscribes from Data Registry callbacks during teardown
void UBmrGeneratedMapSubsystem::Deinitialize()
{
	if (UDalRegistrySubsystem* DalRegistry = UDalRegistrySubsystem::GetDalRegistrySubsystem())
	{
		DalRegistry->Unbind(this);
	}

	Super::Deinitialize();
}

// Called when level actor Data Registry rows change and all their soft references finish async loading
void UBmrGeneratedMapSubsystem::OnLevelActorRowsChanged_Implementation()
{
	TryBroadcastGeneratedMapReady();
}