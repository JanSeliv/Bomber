// Copyright (c) Yevhenii Selivanov

#include "AI/NewAIEndQueryContexts.h"
//---
#include "LevelActors/PlayerCharacter.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIEndQueryContexts)

void UNewAIEndQueryContext_Level::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	const FVector ResultingLocation = UCellsUtilsLibrary::GetCenterCellOnLevel();
	UEnvQueryItemType_Point::SetContextHelper(ContextData, ResultingLocation);
}

void UNewAIEndQueryContext_AllCellsOnLevel::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	TArray<FVector> ContextLocations = FCell::CellsToVectors(UCellsUtilsLibrary::GetAllCellsOnLevel());
	for (FVector& It : ContextLocations)
	{
		It.Z += FCell::CellSize;
	}
	UEnvQueryItemType_Point::SetContextHelper(ContextData, MoveTemp(ContextLocations));
}

void UNewAIEndQueryContext_Player::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	const APlayerCharacter* PlayerCharacter = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	ensureMsgf(PlayerCharacter, TEXT("ASSERT: [%i] %s:\n'PlayerCharacter' condition is FALSE"), __LINE__, *FString(__FUNCTION__));
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, PlayerCharacter);
}
