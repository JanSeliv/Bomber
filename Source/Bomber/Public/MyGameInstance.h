// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
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
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	FVector LevelMapScale = FVector::ZeroVector;

	/** A player nickname*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	FText Nickname = DEFAULT_NICKNAME;
};
