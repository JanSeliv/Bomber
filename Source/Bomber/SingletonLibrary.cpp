// Fill out your copyright notice in the Description page of Project Settings.

#include "SingletonLibrary.h"

#include "Bomber.h"
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
			UE_LOG_STR("ClearOwnerTextRenders: %s destroyed", TextRendersArray[i]);
			TextRendersArray[i]->DestroyComponent();
		}
	}
}

void USingletonLibrary::AddDebugTextRenders_Implementation(
	AActor* Owner,
	const TSet<FCell>& Cells,
	bool& bOutHasCoordinateRenders,
	TArray<UTextRenderComponent*>& OutTextRenderComponents,
	const FLinearColor& TextColor,
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

	bOutHasCoordinateRenders = (CoordinatePosition != FVector(0.f) && RenderText.IsEmpty() == false);
	OutTextRenderComponents.SetNum(bOutHasCoordinateRenders ? Cells.Num() * 2 : Cells.Num());
	for (UTextRenderComponent*& TextRenderIt : OutTextRenderComponents)
	{
		TextRenderIt = NewObject<UTextRenderComponent>(Owner);
		TextRenderIt->RegisterComponent();
	}
	UE_LOG(LogTemp, Warning, TEXT("%s's num of renders: %d (%s)"), *RenderText.ToString(), Owner->GetComponentsByClass(UTextRenderComponent::StaticClass()).Num(), *CoordinatePosition.ToString());
}

#endif

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
	if (IS_PIE(World) == true)  // for editor only
	{
		// Debug IS_VALID(Obj) and IS_TRANSIENT(Obj) macros
		int Counter = 0;
		AGeneratedMap* Obj = GetSingleton()->LevelMap_;
		if (IsValid(Obj))
		{
			++Counter;  // #1
			if ((Obj)->IsValidLowLevel())
			{
				++Counter;  // #2
				if (!Obj->HasAllFlags(RF_Transient))
				{
					++Counter;  // #3
					if (UGameplayStatics::GetCurrentLevelName(Obj->GetWorld()) != "Transient")
					{
						return GetSingleton()->LevelMap_;
					}
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Exit state: %i"), Counter);

		// Find and update the Level Map
		TArray<AActor*> LevelMapArray;
		UGameplayStatics::GetAllActorsOfClass(World, AGeneratedMap::StaticClass(), LevelMapArray);
		if (LevelMapArray.Num() > 0)
		{
			GetSingleton()->LevelMap_ = Cast<AGeneratedMap>(LevelMapArray[0]);
			UE_LOG_STR("PIE:SingletonLibrary:GetLevelMap: %s UPDATED", LevelMapArray[0]);
		}
		else
		{
			return nullptr;
		}
	}
#endif

	return GetSingleton()->LevelMap_;
}
