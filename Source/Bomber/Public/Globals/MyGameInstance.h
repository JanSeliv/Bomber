// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Engine/GameInstance.h"
//---
#include "MyGameInstance.generated.h"

/**
 * Contains a data of standalone and PIE games
 */
UCLASS()
class UMyGameInstance final : public UGameInstance
{
	GENERATED_BODY()

public:
	/** A size of the level map */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	FVector LevelMapScale = FVector::ZeroVector; //[B]
};
