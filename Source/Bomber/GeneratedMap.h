// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GeneratedMap.generated.h"

UENUM(BlueprintType)
enum class EPathTypesEnum : uint8
{
	Explosion,
	Free,
	Safe,
	Secure
};

UENUM(BlueprintType)
enum class EActorTypeEnum : uint8
{
	None,
	Bomb,
	Item,
	Wall,
	Floor,
	Box,
	Player
};

USTRUCT(BlueprintType)
struct FCell
{
	GENERATED_BODY()

public:
	FCell() {}
	FCell(FVector vector);

	UPROPERTY(BlueprintReadWrite, Category = "C++")
		FVector cell;

	// Uses USTUCT in TSet
	bool operator== (const FCell& other)
	{
		return cell == other.cell;
	}
	friend uint32 GetTypeHash(const FCell& other)
	{
		return GetTypeHash(other.cell);
	}
};

UCLASS()
class BOMBER_API AGeneratedMap : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeneratedMap();

	// Pathfinding
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		TSet<FCell> GetSidesCells(FCell cellLocation, int32 sideLength, EPathTypesEnum pathfinder) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		AActor* AddActorOnMap(FCell cellLocation, AActor* updateActor, EActorTypeEnum actorType);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
