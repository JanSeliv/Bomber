// Fill out your copyright notice in the Description page of Project Settings.

#include "SingletonLibrary.h"

#include "Components/TextRenderComponent.h"
#include "Engine.h"
#include "Math/Color.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "Kismet/GameplayStatics.h"
#include "MapComponent.h"
#include "MyAIController.h"
#include "MyCharacter.h"
#include "MyGameInstance.h"

#if WITH_EDITOR		 // [Editor]
#include "Editor.h"  // GEditor
#endif				 //WITH_EDITOR [Editor]

/* ---------------------------------------------------
 *			Editor development functions
* --------------------------------------------------- */

void USingletonLibrary::BroadcastActorsUpdating()
{
#if WITH_EDITOR  // [IsEditorNotPieWorld]

	if (IsEditorNotPieWorld())  // [IsEditorNotPieWorld] only!!!
	{
		PrintToLog(GetSingleton(), "----- [IsEditorNotPieWorld]OnActorsUpdatedDelegate ----->", "RerunConstructionScripts");
		GetSingleton()->OnActorsUpdatedDelegate.Broadcast();
	}

#endif  // WITH_EDITOR [IsEditorNotPieWorld]
}

void USingletonLibrary::BroadcastAiUpdating()
{
#if WITH_EDITOR  // [Editor]

	const auto LevelMap = GetSingleton()->LevelMap_;
	if (LevelMap == nullptr)  // The Level map is null
	{
		return;
	}

	for (AMyCharacter* MyCharacterIt : LevelMap->CharactersOnMap)
	{
		if (MyCharacterIt && MyCharacterIt->MapComponent  // is accessible
			&& MyCharacterIt->MapComponent->bShouldShowRenders)
		{
			auto MyAIController = Cast<AMyAIController>(MyCharacterIt->GetController());
			if (MyAIController)
			{
				FCell OutCell;					 // temporary param
				TSet<FCell> OutFree{};			 // temporary param
				bool bOutIsDangerous;			 // temporary param
				TSet<FCell> OutAllCrossways;	 ///< temporary param
				TSet<FCell> OutSecureCrossways;  ///< temporary param
				TSet<FCell> OutFoundItems;		 ///< temporary param
				bool bOutIsItemInDirect;		 ///< temporary param
				TSet<FCell> OutFiltered;		 ///< temporary param
				bool bIsFilteringFailed;		 ///< temporary param
				MyAIController->UpdateAI(OutCell, OutFree, bOutIsDangerous, OutAllCrossways, OutSecureCrossways, OutFoundItems, bOutIsItemInDirect, OutFiltered, bIsFilteringFailed);
			}
		}
	}

#endif  // WITH_EDITOR [Editor]
}

bool USingletonLibrary::IsEditorNotPieWorld()
{
#if WITH_EDITOR  // [IsEditorNotPieWorld]
	if (GIsEditor && GEditor)
	{
		return GEditor->GetEditorWorldContext().World() == GWorld;
		// return GEditor->GetPIEWorldContext() == nullptr;
	}
#endif  // [IsEditorNotPieWorld]
	return false;
}

void USingletonLibrary::ClearOwnerTextRenders(AActor* Owner)
{
#if WITH_EDITOR  // [Editor]

	if (IS_VALID(Owner) == false)  // The owner is not valid
	{
		return;
	}

	const TArray<UActorComponent*> TextRendersArray = Owner->GetComponentsByClass(UTextRenderComponent::StaticClass());
	if (TextRendersArray.Num() > 0)
	{
		for (int32 i = TextRendersArray.Num() - 1; i >= 0; --i)
		{
			FString StringIt = Cast<UTextRenderComponent>(TextRendersArray[i])->Text.ToString();
			if (StringIt != "Player" && StringIt != "AI")  // is not nickname
			{
				TextRendersArray[i]->DestroyComponent();
			}
		}

		if (IsEditorNotPieWorld()) PrintToLog(Owner, "[IsEditorNotPieWorld]ClearOwnerTextRenders \t Components removed:", FString::FromInt(TextRendersArray.Num()));
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

	if (Cells.Num() == NULL			  // Null length
		|| !IS_VALID(Owner)			  // Owner is not valid
		|| !IS_VALID(GetLevelMap()))  // The Level Map is not valid
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

	if (IsEditorNotPieWorld()) PrintToLog(Owner, "[IsEditorNotPieWorld]AddDebugTextRenders \t added renders:", *(FString::FromInt(OutTextRenderComponents.Num()) + RenderText.ToString() + FString(bOutHasCoordinateRenders ? "\t Double" : "")));

#endif  // WITH_EDITOR [Editor]
}

#if WITH_EDITOR  // [Editor] AddDebugTextRenders()
void USingletonLibrary::AddDebugTextRenders(AActor* Owner, const TSet<FCell>& Cells, const FLinearColor& TextColor, float TextHeight, float TextSize, const FString& RenderString, const FVector& CoordinatePosition)
{
	bool bOutBool = false;
	TArray<UTextRenderComponent*> OutArray{};
	GetSingleton()->AddDebugTextRenders(Owner, Cells, TextColor, bOutBool, OutArray, TextHeight, TextSize, FText::FromString(RenderString), CoordinatePosition);
}
#endif  // WITH_EDITOR [Editor]

/* ---------------------------------------------------
 *			Static library functions
 * --------------------------------------------------- */

USingletonLibrary* USingletonLibrary::GetSingleton()
{
	USingletonLibrary* Singleton = nullptr;
	if (GEngine) Singleton = Cast<USingletonLibrary>(GEngine->GameSingleton);
	ensureMsgf(Singleton != nullptr, TEXT("The Singleton is null"));
	return Singleton;
}

AGeneratedMap* USingletonLibrary::GetLevelMap()
{
	if (GetSingleton() == nullptr)
	{
		return nullptr;
	}

#if WITH_EDITOR  // [IsEditorNotPieWorld]

	if (IsEditorNotPieWorld() == true						 // IsEditorNotPieWorld only
		&& (GetSingleton()->LevelMap_ == nullptr			 // is nullptr
			   || IS_TRANSIENT(GetSingleton()->LevelMap_)))  // Is transient
	{
		SetLevelMap(nullptr);  // Find the Level Map
	}
#endif  // WITH_EDITOR [IsEditorNotPieWorld]

	return GetSingleton()->LevelMap_.Get();
}

UMyGameInstance* USingletonLibrary::GetMyGameInstance(const UObject* WorldContextObject)
{
	return Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
}

void USingletonLibrary::SetLevelMap(AGeneratedMap* LevelMap)
{
#if WITH_EDITOR  // [IsEditorNotPieWorld]

	if (IsEditorNotPieWorld()  // IsEditorNotPieWorld only
		&& LevelMap == nullptr)
	{
		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		if (EditorWorld)
		{
			TArray<AActor*> LevelMapsArray;
			UGameplayStatics::GetAllActorsOfClass(EditorWorld, AGeneratedMap::StaticClass(), LevelMapsArray);
			if (LevelMapsArray.Num() > 0)
			{
				LevelMap = Cast<AGeneratedMap>(LevelMapsArray[0]);
			}
		}
	}
#endif  // WITH_EDITOR [IsEditorNotPieWorld]

	if (ensureMsgf(IS_VALID(LevelMap), TEXT("ERROR: SetLevelMap: The Level Map is not valid!")))
	{
		GetSingleton()->LevelMap_ = LevelMap;
		PrintToLog(LevelMap, "----- SetLevelMap", "UPDATED -----");
	}
}

/* ---------------------------------------------------
 *			FCell blueprint functions
 * --------------------------------------------------- */

FCell USingletonLibrary::MakeCell_Implementation(const AActor* Actor) const
{
	return FCell::ZeroCell;
}

FCell USingletonLibrary::CalculateVectorAsRotatedCell(const FVector& VectorToRotate, const float& AxisZ)
{
	const AGeneratedMap* LevelMap = GetLevelMap();
	if (!ensureMsgf(LevelMap, TEXT("The Level Map is not valid"))  //
		|| !ensureMsgf(AxisZ != abs(0.f), TEXT("The axis is zero")))
	{
		return FCell(VectorToRotate);
	}

	const FVector Dimensions = VectorToRotate - LevelMap->GetActorLocation();
	const FVector RotatedVector = Dimensions.RotateAngleAxis(LevelMap->GetActorRotation().Yaw, FVector(0, 0, AxisZ));
	return FCell(VectorToRotate + RotatedVector - Dimensions);
}

/* ---------------------------------------------------
 *			EActorTypeEnum bitmask functions
 * --------------------------------------------------- */

bool USingletonLibrary::IsActorInTypes(const AActor* Actor, const int32& Bitmask)
{
	if (IS_VALID(Actor) == false) return false;  // The actor is null

	const EActorTypeEnum* FoundActorType = GetSingleton()->ActorTypesByClasses_.FindKey(Actor->GetClass());
	if (FoundActorType == nullptr) return false;

	return ContainsActorType(*FoundActorType, Bitmask);
}

TSubclassOf<AActor> USingletonLibrary::FindClassByActorType(const EActorTypeEnum& ActorType)
{
	if (ActorType != EActorTypeEnum::None)
	{
		const TSubclassOf<AActor>* ActorClass = GetSingleton()->ActorTypesByClasses_.Find(ActorType);
		if (ActorClass != nullptr)
		{
			return *ActorClass;
		}
	}
	return nullptr;
}
