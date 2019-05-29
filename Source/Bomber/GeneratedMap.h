// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GeneratedMap.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EPathTypesEnum : uint8
{
	Explosion = 1 << 0, // to the first wall
	Free = 1 << 1,		// to the first obstacle
	Safe = 1 << 2,		// to the first obstacle+explosion
	Secure = 1 << 3		// to the first obstacle+explosion+player
};

UENUM(BlueprintType, meta = (Bitflags))
enum class EActorTypeEnum : uint8
{
	None = 1 << 0,
	Bomb = 1 << 1,  // obstacle
	Item = 1 << 2,
	Wall = 1 << 3,	// obstacle
	Floor = 1 << 4,
	Box = 1 << 5,	// obstacle
	Player = 1 << 6
};

USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

public:
	FCell();

	UPROPERTY(BlueprintReadWrite, Category = "C++")
		FVector location;

	bool operator== (const FCell& other)
	{
		return (this->location == other.location);
	}
	// Uses USTUCT in TSet
	// Hash Function
	friend uint32 GetTypeHash(const FCell& other)
	{
		return GetTypeHash(other.location);
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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++")
		TSet<FCell> GetSidesCells(const FCell& cell, int32 sideLength, EPathTypesEnum pathfinder) const;

	// Return TSet of actor cells by types
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (AdvancedDisplay = 2))
		TSet<FCell> FilterCellsByTypes(const TSet<FCell>& keys, const TArray<EActorTypeEnum>& filterTypes, const ACharacter* excludePlayer) const;

	// Spawn or update actor by type on cell
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		AActor* AddActorOnMap(const FCell& cell, AActor* updateActor, EActorTypeEnum actorType);

	// Delete actor from cell and TMap
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		bool DestroyActorFromMap(const FCell& cell);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Create LevelMap on Scene and fill TMap
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		void GenerateLevelMap();

	// Storage of cells and their actors
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++", meta = (DisplayName = "Generated Map"))
		TMap<FCell, AActor*> GeneratedMap_;
};