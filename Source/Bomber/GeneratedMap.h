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
	Wall = 1 << 0,	// obstacle
	Box = 1 << 1,	// obstacle
	Bomb = 1 << 2,  // obstacle
	Item = 1 << 3,
	Floor = 1 << 4,
	Player = 1 << 5
};

USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

public:
	FCell() {};

	FCell(const AActor* actor);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FVector location;

	bool operator== (const FCell& other) const
	{
		return (this->location == other.location);
	}
	// Hash Function
	friend FORCEINLINE uint32 GetTypeHash(const FCell& other)
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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPushNongeneratedToMap);
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "C++")
		FPushNongeneratedToMap onActorsUpdateDelegate;

	// Pathfinding
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++")
		TSet<FCell> GetSidesCells(const FCell& cell, int32 sideLength, EPathTypesEnum pathfinder) const;

	// Return TSet of actor cells by types
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (AdvancedDisplay = 2))
		TSet<FCell> FilterCellsByTypes(const TSet<FCell>& keys, const TArray<EActorTypeEnum>& filterTypes, const ACharacter* excludePlayer) const;

	// Spawn or update actor by type on cell
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		AActor* AddActorOnMap(const FCell& cell, EActorTypeEnum actorType);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		void AddActorOnMapByObj(const AActor* updateActor);

	// Delete actor from cell and TMap
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		void DestroyActorFromMap(const FCell& cell);

protected:
	friend FCell;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) override;

	// Create LevelMap on Scene and fill TMap
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
		void GenerateLevelMap();

	// Storage of cells and their actors
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
		TMap<FCell, AActor*> GeneratedMap_;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++", meta = (DisplayName = "Grid Array"))
		TSet<ACharacter*> charactersOnMap_;


};