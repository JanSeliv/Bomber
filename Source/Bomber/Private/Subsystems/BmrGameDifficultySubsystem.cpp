// Copyright (c) Yevhenii Selivanov

#include "Subsystems/BmrGameDifficultySubsystem.h"

// Bomber
#include "DataRegistries/BmrGameDifficultyRow.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameDifficultySubsystem)

// Returns this subsystem, is checked and will crash if can't be obtained
UBmrGameDifficultySubsystem& UBmrGameDifficultySubsystem::Get(const UObject* WorldContextObject /* = nullptr*/)
{
	UBmrGameDifficultySubsystem* Subsystem = GetGameDifficultySubsystem(WorldContextObject);
	checkf(Subsystem, TEXT("ERROR: [%i] %hs:\n'Subsystem' is null!"), __LINE__, __FUNCTION__);
	return *Subsystem;
}

// Returns the pointer to this subsystem
UBmrGameDifficultySubsystem* UBmrGameDifficultySubsystem::GetGameDifficultySubsystem(const UObject* WorldContextObject /* = nullptr*/)
{
	const UWorld* FoundWorld = UUtilsLibrary::GetPlayWorld(WorldContextObject);
	return FoundWorld ? FoundWorld->GetSubsystem<UBmrGameDifficultySubsystem>() : nullptr;
}

/*********************************************************************************************
 * Difficulty Tag API
 ********************************************************************************************* */

// Returns current difficulty tag from ASC, falls back to config if ASC unavailable
FBmrGameDifficultyTag UBmrGameDifficultySubsystem::GetGameDifficultyTag() const
{
	const UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	const FGameplayTagContainer DifficultyTags = UGameplayUtilsLibrary::GetFilteredGameplayTags(ASC, FBmrGameDifficultyTag::ParentTag);
	return DifficultyTags.IsEmpty() ? DifficultyTag : FBmrGameDifficultyTag(DifficultyTags.First());
}

// Sets new difficulty tag on ASC, adds new before removing old to prevent unnecessary GFP unload/reload
void UBmrGameDifficultySubsystem::SetGameDifficultyTag(FBmrGameDifficultyTag NewTag)
{
	const UWorld* World = GetWorld();
	if (!World
	    || World->IsNetMode(NM_Client)
	    || !ensureMsgf(NewTag.IsValid(), TEXT("ASSERT: [%i] %hs:\n'NewTag' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__)
	    || ASC->HasMatchingGameplayTag(NewTag))
	{
		return;
	}

	// Save to config
	DifficultyTag = NewTag;
	SaveConfig();

	// Remove old difficulty tags
	const FGameplayTagContainer OldTags = UGameplayUtilsLibrary::GetFilteredGameplayTags(ASC, FBmrGameDifficultyTag::ParentTag);
	for (const FGameplayTag& OldTag : OldTags)
	{
		ASC->RemoveLooseGameplayTag(OldTag, 1, EGameplayTagReplicationState::TagOnly);
	}

	// Apply new tag
	ASC->AddLooseGameplayTag(NewTag, 1, EGameplayTagReplicationState::TagOnly);
}

// Returns current difficulty level integer from Data Registry row lookup
int32 UBmrGameDifficultySubsystem::GetDifficultyLevel() const
{
	const FBmrGameDifficultyTag CurrentTag = GetGameDifficultyTag();
	const FBmrGameDifficultyRow* Row = FindRowByTag(CurrentTag);
	return Row ? Row->DifficultyLevel : INDEX_NONE;
}

// Sets difficulty by integer level, converts to tag via Data Registry row lookup
void UBmrGameDifficultySubsystem::SetDifficultyLevel(int32 InLevel)
{
	const UWorld* World = GetWorld();
	if (!World
	    || World->IsNetMode(NM_Client))
	{
		return;
	}

	const FBmrGameDifficultyRow* Row = FindRowByLevel(InLevel);
	if (!ensureMsgf(Row, TEXT("ASSERT: [%i] %hs:\nNo difficulty row found for level %i!"), __LINE__, __FUNCTION__, InLevel))
	{
		return;
	}

	SetGameDifficultyTag(Row->DifficultyTag);
}

// Populates combobox display names from all Data Registry difficulty rows sorted by DifficultyLevel
void UBmrGameDifficultySubsystem::GetAllDifficultyDisplayTexts(TArray<FText>& OutMembers) const
{
	TArray<const FBmrGameDifficultyRow*> Rows;
	FBmrGameDifficultyRow::GetRowsByPredicate(Rows, [](const FBmrGameDifficultyRow&)
	{
		return true;
	});

	Rows.Sort([](const FBmrGameDifficultyRow& A, const FBmrGameDifficultyRow& B)
	{
		return A.DifficultyLevel < B.DifficultyLevel;
	});

	OutMembers.Reset(Rows.Num());
	for (const FBmrGameDifficultyRow* Row : Rows)
	{
		if (Row)
		{
			OutMembers.Emplace(Row->DisplayName);
		}
	}
}

/*********************************************************************************************
 * Data Registry Row Lookup
 ********************************************************************************************* */

// Finds difficulty row by tag, returns nullptr if not found
const FBmrGameDifficultyRow* UBmrGameDifficultySubsystem::FindRowByTag(const FBmrGameDifficultyTag& Tag)
{
	return FBmrGameDifficultyRow::GetRowByPredicate([&Tag](const FBmrGameDifficultyRow& Row)
	{
		return Row.DifficultyTag == Tag;
	});
}

// Finds difficulty row by level, returns nullptr if not found
const FBmrGameDifficultyRow* UBmrGameDifficultySubsystem::FindRowByLevel(int32 Level)
{
	return FBmrGameDifficultyRow::GetRowByPredicate([Level](const FBmrGameDifficultyRow& Row)
	{
		return Row.DifficultyLevel == Level;
	});
}

/*********************************************************************************************
 * Event Broadcasting
 ********************************************************************************************* */

// Broadcasts Event::Difficulty_Changed with current difficulty tag in InstigatorTags
void UBmrGameDifficultySubsystem::BroadcastDifficultyChanged()
{
	const UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = BmrGameplayTags::Event::Difficulty_Changed;
	Payload.InstigatorTags = UGameplayUtilsLibrary::GetFilteredGameplayTags(ASC, FBmrGameDifficultyTag::ParentTag);
	UGlobalMessageSubsystem::BroadcastGlobalMessage(Payload);
}

// Subscribes to ASC generic tag event, filters for Difficulty.* tags
void UBmrGameDifficultySubsystem::BindOnDifficultyTagChanged()
{
	UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("ASSERT: [%i] %hs:\n'ASC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	ASC->RegisterGenericGameplayTagEvent().AddWeakLambda(this, [this](const FGameplayTag Tag, int32 NewCount)
	{
		if (NewCount > 0
		    && Tag.MatchesTag(FBmrGameDifficultyTag::ParentTag)
		    && Tag != FBmrGameDifficultyTag::ParentTag)
		{
			BroadcastDifficultyChanged();
		}
	});
}

// Called when the world ASC becomes available, subscribes to ASC difficulty tag events
void UBmrGameDifficultySubsystem::OnWorldASCReady_Implementation(const FGameplayEventData& Payload)
{
	BindOnDifficultyTagChanged();

	// Apply config difficulty tag
	if (DifficultyTag.IsValid())
	{
		SetGameDifficultyTag(DifficultyTag);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Subscribes to WorldASC_Ready for ASC tag binding
void UBmrGameDifficultySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::WorldASC_Ready, this, &ThisClass::OnWorldASCReady);
}

// Cleans up ASC bindings
void UBmrGameDifficultySubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	if (UAbilitySystemComponent* ASC = UBmrBlueprintFunctionLibrary::GetWorldAbilitySystemComponent())
	{
		ASC->RegisterGenericGameplayTagEvent().RemoveAll(this);
	}

	Super::OnWorldEndPlay(InWorld);
}