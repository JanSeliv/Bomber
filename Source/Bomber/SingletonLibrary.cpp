// Fill out your copyright notice in the Description page of Project Settings.

#include "SingletonLibrary.h"

#include "Bomber.h"
#include "Components/TextRenderComponent.h"
#include "Engine.h"
#include "GeneratedMap.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Color.h"
#include "MyAiCharacter.h"

void USingletonLibrary::BroadcastAiUpdating()
{
#if WITH_EDITOR  // [Editor]
	const auto LevelMap = GetSingleton()->LevelMap_;
	if (LevelMap == nullptr)  // The Level map is null
	{
		return;
	}

	for (AMyCharacter* const MyCharacterIt : LevelMap->CharactersOnMap)
	{
		AMyAiCharacter* const MyAiCharacter = Cast<AMyAiCharacter>(MyCharacterIt);
		if (MyAiCharacter != nullptr					   // Successfully cast to AI
			&& MyAiCharacter->bShouldShowRenders == true)  // Is render AI
		{
			MyAiCharacter->UpdateAI();
		}
	}
#endif  // WITH_EDITOR [Editor]
}

void USingletonLibrary::ClearOwnerTextRenders(AActor* Owner)
{
#if WITH_EDITOR					   // [Editor]
	if (IS_VALID(Owner) == false)  // The owner is not valid
	{
		return;
	}

	const TArray<UActorComponent*> TextRendersArray = Owner->GetComponentsByClass(UTextRenderComponent::StaticClass());
	if (TextRendersArray.Num() > 0)
	{
		for (int32 i = TextRendersArray.Num() - 1; i >= 0; --i)
		{
			TextRendersArray[i]->DestroyComponent();
		}

		if (IS_PIE(Owner->GetWorld())) PrintToLog(Owner, "[Editor]ClearOwnerTextRenders \t Components removed:", FString::FromInt(TextRendersArray.Num()));
	}
#endif  // WITH_EDITOR [Editor]
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
#if WITH_EDITOR  // [Editor]
	AMyAiCharacter* const MyAiCharacter = Cast<AMyAiCharacter>(Owner);
	if ((MyAiCharacter != nullptr							// Successfully cast to AI
			&& MyAiCharacter->bShouldShowRenders == false)  // Is not render AI
		|| Cells.Num() == NULL								// Null length
		|| IS_VALID(Owner) == false							// Owner is not valid
		|| !IS_VALID(GetLevelMap()))						// The Level Map is not valid
	{
		return;
	}

	bOutHasCoordinateRenders = (CoordinatePosition.IsZero() == false && RenderText.IsEmpty() == false);
	OutTextRenderComponents.SetNum(bOutHasCoordinateRenders ? Cells.Num() * 2 : Cells.Num());
	for (UTextRenderComponent*& TextRenderIt : OutTextRenderComponents)
	{
		TextRenderIt = NewObject<UTextRenderComponent>(Owner);
		TextRenderIt->RegisterComponent();
		//TextRenderIt->MarkAsEditorOnlySubobject();
	}

	if (IS_PIE(Owner->GetWorld())) PrintToLog(Owner, "[Editor]AddDebugTextRenders \t added renders:", *(FString::FromInt(OutTextRenderComponents.Num()) + RenderText.ToString() + FString(bOutHasCoordinateRenders ? "\t Double" : "")));
#endif  // WITH_EDITOR [Editor]
}

#if WITH_EDITOR  // [Editor] AddDebugTextRenders()
void USingletonLibrary::AddDebugTextRenders(AActor* Owner, const TSet<FCell>& Cells, const FLinearColor& TextColor)
{
	bool bOutBool = false;
	TArray<class UTextRenderComponent*> OutArray{};
	GetSingleton()->AddDebugTextRenders(Owner, Cells, TextColor, bOutBool, OutArray);
}
#endif  // WITH_EDITOR [Editor]

FCell USingletonLibrary::MakeCell_Implementation(const AActor* Actor) const
{
	return FCell::ZeroCell;
}

USingletonLibrary* USingletonLibrary::GetSingleton()
{
	USingletonLibrary* Singleton = nullptr;
	if (GEngine) Singleton = Cast<USingletonLibrary>(GEngine->GameSingleton);
	ensureMsgf(Singleton != nullptr, TEXT("The Singleton is null"));
	return Singleton;
}

FCell USingletonLibrary::CalculateVectorAsRotatedCell(const FVector& VectorToRotate, const float& AxisZ)
{
	if (!ensureMsgf(IS_VALID(GetSingleton()->LevelMap_), TEXT("The Level Map is not valid"))  //
		|| !ensureMsgf(AxisZ != abs(0.f), TEXT("The axis is zero")))
	{
		return FCell::ZeroCell;
	}

	const FVector Dimensions = VectorToRotate - GetSingleton()->LevelMap_->GetActorLocation();
	const FVector RotatedVector = Dimensions.RotateAngleAxis(GetSingleton()->LevelMap_->GetActorRotation().Yaw, FVector(0, 0, AxisZ));
	return FCell(VectorToRotate + RotatedVector - Dimensions);
}

bool USingletonLibrary::IsActorInTypes(const AActor* Actor, const int32& Bitmask)
{
	if (IS_VALID(Actor) == false) return false;  // The actor is null

	const EActorTypeEnum* FoundActorType = GetSingleton()->ActorTypesByClasses.FindKey(Actor->GetClass());
	if (FoundActorType == nullptr) return false;

	return ContainsActorType(*FoundActorType, Bitmask);
}

TSubclassOf<AActor> USingletonLibrary::FindClassByActorType(const EActorTypeEnum& ActorType)
{
	if (ActorType != EActorTypeEnum::None)
	{
		const TSubclassOf<AActor>* ActorClass = GetSingleton()->ActorTypesByClasses.Find(ActorType);
		if (ActorClass != nullptr)
		{
			return *ActorClass;
		}
	}
	return nullptr;
}
