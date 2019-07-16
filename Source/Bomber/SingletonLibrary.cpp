// Fill out your copyright notice in the Description page of Project Settings.

#include "SingletonLibrary.h"

#include "Components/TextRenderComponent.h"

#include "GeneratedMap.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Color.h"
#include "MyAiCharacter.h"

#if WITH_EDITOR
void USingletonLibrary::BroadcastAiUpdating(AActor* Owner)
{
	if (LevelMap_ == nullptr)  // The Level map is null
	{
		return;
	}

	for (AMyCharacter* const MyCharacterIt : LevelMap_->CharactersOnMap)
	{
		AMyAiCharacter* const MyAiCharacter = Cast<AMyAiCharacter>(MyCharacterIt);
		if (MyAiCharacter != nullptr					   // Successfully cast to AI
			&& MyAiCharacter->bShouldShowRenders == true)  // Is render AI
		{
			MyAiCharacter->UpdateAI();
		}
	}
}

void USingletonLibrary::ClearOwnerTextRenders(AActor* Owner)
{
	const TArray<UActorComponent*> TextRendersArray = Owner->GetComponentsByClass(UTextRenderComponent::StaticClass());
	if (TextRendersArray.Num() > 0)
	{
		for (int32 i = TextRendersArray.Num() - 1; i >= 0; --i)
		{
			TextRendersArray[i]->DestroyComponent();
		}
		UE_LOG_STR(Owner, "[Dev]ClearOwnerTextRenders \t Components removed:", FString::FromInt(TextRendersArray.Num()));
	}
}

void USingletonLibrary::AddDebugTextRenders_Implementation(
	AActor* Owner,
	const TSet<FCell>& Cells,
	const FLinearColor& TextColor,
	bool& bOutHasCoordinateRenders,
	TArray<UTextRenderComponent*>& OutTextRenderComponents,
	float TextHeight,
	float TextSize,
	const FText& RenderText,
	const FVector& CoordinatePosition) const
{
	AMyAiCharacter* const MyAiCharacter = Cast<AMyAiCharacter>(Owner);
	if ((MyAiCharacter != nullptr							// Successfully cast to AI
			&& MyAiCharacter->bShouldShowRenders == false)  // Is not render AI
		|| Cells.Num() == NULL								// Null length
		|| IS_VALID(Owner) == false)						// Owner is not valid
	{
		return;
	}

	bOutHasCoordinateRenders = (CoordinatePosition.IsZero() == false && RenderText.IsEmpty() == false);
	OutTextRenderComponents.SetNum(bOutHasCoordinateRenders ? Cells.Num() * 2 : Cells.Num());
	for (UTextRenderComponent*& TextRenderIt : OutTextRenderComponents)
	{
		TextRenderIt = NewObject<UTextRenderComponent>(Owner);
		TextRenderIt->RegisterComponent();
	}
	UE_LOG_STR(Owner, "[Dev]AddDebugTextRenders \t added renders:", *(FString::FromInt(OutTextRenderComponents.Num()) + RenderText.ToString() + FString(bOutHasCoordinateRenders ? "\t Double" : "")));
}

#endif  // WITH_EDITOR [Dev]

AGeneratedMap* const USingletonLibrary::GetLevelMap(UObject* WorldContextObject)
{
	if (GEngine == nullptr				   // Global engine pointer is null
		|| GetSingleton() == nullptr	   // Singleton is null
		|| WorldContextObject == nullptr)  // WorldContext is null
	{
		return nullptr;
	}

// Find editor level map
#if WITH_EDITOR
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (IS_PIE(World) == true  // for editor only
		&& IS_VALID(GetSingleton()->LevelMap_) == false)
	{
		// Find and update the Level Map
		TArray<AActor*> LevelMapArray;
		UGameplayStatics::GetAllActorsOfClass(World, AGeneratedMap::StaticClass(), LevelMapArray);
		if (LevelMapArray.Num() > 0)
		{
			GetSingleton()->LevelMap_ = Cast<AGeneratedMap>(LevelMapArray[0]);
			UE_LOG_STR(LevelMapArray[0], "[PIE]SingletonLibrary:GetLevelMap", "UPDATED");
		}
		else
		{
			return nullptr;
		}
	}
#endif  //WITH_EDITOR [PIE]

	return GetSingleton()->LevelMap_;
}
