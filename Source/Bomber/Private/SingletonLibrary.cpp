// Copyright 2020 Yevhenii Selivanov.

#include "SingletonLibrary.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "LevelActorDataAsset.h"
#include "MapComponent.h"
#include "MyGameInstance.h"
#include "MyGameModeBase.h"
//---
#include "Components/TextRenderComponent.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Color.h"
//---
#if WITH_EDITOR		 // [Editor]
#include "Editor.h"	 // GEditor
#endif				 //WITH_EDITOR [Editor]

/* ---------------------------------------------------
 *		Editor development functions
 * --------------------------------------------------- */

// Binds to update movements of each AI controller.
USingletonLibrary::FUpdateAI USingletonLibrary::GOnAIUpdatedDelegate;

// Checks, that this actor placed in the editor world and the game is not started yet
bool USingletonLibrary::IsEditorNotPieWorld()
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (GIsEditor && GEditor)
	{
		return GEditor->GetEditorWorldContext().World() == GWorld;
	}
#endif	// [IsEditorNotPieWorld]
	return false;
}

void USingletonLibrary::PrintToLog(const UObject* UObj, const FString& FunctionName, const FString& Message)
{
#if WITH_EDITOR	 // [Editor]
	AGeneratedMap* const LevelMap = GetLevelMap();
	if (!LevelMap || LevelMap->bShouldShowRenders)	// The Level Map is not accessible or has the debug mode
	{
		UE_LOG(LogTemp, Warning, TEXT("\t %s \t %s \t %s"), (UObj ? *UObj->GetName() : TEXT("nullptr")), *FunctionName, *Message);
	}
#endif	//WITH_EDIT [Editor]
}

// Remove all text renders of the Owner
void USingletonLibrary::ClearOwnerTextRenders(AActor* Owner)
{
#if WITH_EDITOR	 // [Editor]

	if (IS_VALID(Owner) == false)  // The owner is not valid
	{
		return;
	}

	TArray<UActorComponent*> TextRendersArray;
	Owner->GetComponents(UTextRenderComponent::StaticClass(), TextRendersArray);
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

#endif	// WITH_EDITOR [Editor]
}

// Debug visualization by text renders
void USingletonLibrary::AddDebugTextRenders_Implementation(
	AActor* Owner,
	const FCells& Cells,
	const FLinearColor& TextColor,
	bool& bOutHasCoordinateRenders,
	TArray<UTextRenderComponent*>& OutTextRenderComponents,
	float TextHeight,
	float TextSize,
	const FString& RenderString,
	const FVector& CoordinatePosition) const
{
#if WITH_EDITOR	 // [Editor]

	if (Cells.Num() == NULL			  // Null length
		|| !IS_VALID(Owner)			  // Owner is not valid
		|| !IS_VALID(GetLevelMap()))  // The Level Map is not valid
	{
		return;
	}

	bOutHasCoordinateRenders = (CoordinatePosition.IsZero() == false && RenderString.IsEmpty() == false);
	OutTextRenderComponents.SetNum(bOutHasCoordinateRenders ? Cells.Num() * 2 : Cells.Num());
	for (UTextRenderComponent*& TextRenderIt : OutTextRenderComponents)
	{
		TextRenderIt = NewObject<UTextRenderComponent>(Owner);
		TextRenderIt->RegisterComponent();
		//TextRenderIt->MarkAsEditorOnlySubobject();
	}

	if (IsEditorNotPieWorld()) PrintToLog(Owner, "[IsEditorNotPieWorld]AddDebugTextRenders \t added renders:", *(FString::FromInt(OutTextRenderComponents.Num()) + RenderString + FString(bOutHasCoordinateRenders ? "\t Double" : "")));

#endif	// WITH_EDITOR [Editor]
}

// Shortest static overloading of debugging visualization without outer params
#if WITH_EDITOR	 // [Editor] AddDebugTextRenders()
void USingletonLibrary::AddDebugTextRenders(AActor* Owner, const TArray<FSharedCell>& SharedCells, const FLinearColor& TextColor, float TextHeight, float TextSize, const FString& RenderString, const FVector& CoordinatePosition)
{
	bool bOutBool = false;
	TArray<UTextRenderComponent*> OutArray{};
	FCells Cells;
	Cells.Reserve(SharedCells.Num());
	for (const auto& SharedCell : SharedCells)
	{
		Cells.Emplace(*SharedCell);
	}
	GetSingleton()->AddDebugTextRenders(Owner, Cells, TextColor, bOutBool, OutArray, TextHeight, TextSize, RenderString, CoordinatePosition);
}
#endif	// WITH_EDITOR [Editor]

/* ---------------------------------------------------
 *		Static library functions
 * --------------------------------------------------- */

//  Returns the singleton, nullptr otherwise
USingletonLibrary* USingletonLibrary::GetSingleton()
{
	USingletonLibrary* Singleton = nullptr;
	if (GEngine) Singleton = Cast<USingletonLibrary>(GEngine->GameSingleton);
	checkf(Singleton, TEXT("The Singleton is null"));
	return Singleton;
}

// The Level Map getter, nullptr otherwise
AGeneratedMap* USingletonLibrary::GetLevelMap()
{
	if (GetSingleton() == nullptr)
	{
		return nullptr;
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (IsEditorNotPieWorld() == true			  // IsEditorNotPieWorld only
		&& !GetSingleton()->LevelMapInternal.IsValid())  // Is transient
	{
		SetLevelMap(nullptr);  // Find the Level Map
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	return GetSingleton()->LevelMapInternal.Get();
}

// The Level Map setter. If the specified Level Map is not valid or is transient, find and set another one
void USingletonLibrary::SetLevelMap(const AGeneratedMap* LevelMap)
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]

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
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	if (IS_VALID(LevelMap))
	{
		GetSingleton()->LevelMapInternal = LevelMap;
		PrintToLog(LevelMap, "----- SetLevelMap", "UPDATED -----");
	}
}

// Contains a data of standalone and PIE games, nullptr otherwise
UMyGameInstance* USingletonLibrary::GetMyGameInstance(const UObject* WorldContextObject)
{
	return Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
}

// Contains a data of Bomber Level, nullptr otherwise
AMyGameModeBase* USingletonLibrary::GetMyGameMode(const UObject* WorldContextObject)
{
	return Cast<AMyGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
}

/* ---------------------------------------------------
 *		FCell blueprint functions
 * --------------------------------------------------- */

// Find the average of an array of vectors
FCell USingletonLibrary::GetCellArrayAverage(const FCells& Cells)
{
	FVector Sum(0.f);
	FVector Average(0.f);
	const int32 CellsNum = Cells.Num();
	if (CellsNum > 0)
	{
		for (const auto& CellIt : Cells)
		{
			Sum += CellIt.Location;
		}

		Average = Sum / static_cast<float>(CellsNum);
	}

	return FCell(Average);
}

//
ULevelActorDataAsset* USingletonLibrary::GetDataAssetByActorClass(const TSubclassOf<AActor>& ActorClass)
{
	for (ULevelActorDataAsset*& DataAssetIt : GetSingleton()->ActorsDataAssetsInternal)
	{
		if (DataAssetIt && DataAssetIt->GetActorClass()->IsChildOf(ActorClass))
		{
			return DataAssetIt;
		}
	}
	return nullptr;
}

//
ULevelActorDataAsset* USingletonLibrary::GetDataAssetByActorType(EActorType ActorType)
{
	for (ULevelActorDataAsset*& DataAssetIt : GetSingleton()->ActorsDataAssetsInternal)
	{
		if (DataAssetIt && DataAssetIt->GetActorType() == ActorType)
		{
			return DataAssetIt;
		}
	}
	return nullptr;
}

//
TSubclassOf<AActor> USingletonLibrary::GetActorClassByType(EActorType ActorType)
{
	const ULevelActorDataAsset* DataAsset = USingletonLibrary::GetDataAssetByActorType(ActorType);
	return DataAsset ? DataAsset->GetActorClass() : nullptr;
}
