// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Engine/GameInstance.h"
//---
#include "MyGameInstance.generated.h"

#define DEFAULT_NICKNAME FText::FromString(TEXT("Player"))

/**
 * Contains a data of standalone and PIE games
 */
UCLASS()
class BOMBER_API UMyGameInstance final : public UGameInstance
{
	GENERATED_BODY()

public:
	/** A size of the level map */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FVector LevelMapScale = FVector::ZeroVector; //[B]

	/** A player nickname*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FText Nickname = DEFAULT_NICKNAME; //[B]
};
