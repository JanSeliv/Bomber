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
	Wall = 1 << 0, // obstacle
	Box = 1 << 1,  // obstacle
	Bomb = 1 << 2, // obstacle
	Item = 1 << 3,
	Floor = 1 << 4,
	Player = 1 << 5
};

USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

public:
	FCell(){};

	FCell(const AActor* actor);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector location;

	bool operator==(const FCell& other) const
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
	FPushNongeneratedToMap onActorsUpdatedDelegate;

	// Pathfinding
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++")
	TSet<FCell> GetSidesCells(const FCell& cell, int32 sideLength, EPathTypesEnum pathfinder) const;

	// Return TSet of actor cells by types
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (AdvancedDisplay = 2))
	TSet<FCell> FilterCellsByTypes(const TSet<FCell>& keys, const TArray<EActorTypeEnum>& filterTypes, const ACharacter* excludePlayer) const;

	// Spawn actor by type
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	AActor* AddActorOnMap(const FCell& cell, EActorTypeEnum actorType);

	// Blueprint-overriding AddActorOnMap, update actor by obj
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void AddActorOnMapByObj(const FCell& cell, const AActor* updateActor);

	// Destro all actors from scene by cell
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void DestroyActorsFromMap(const FCell& cell);

	// Storage of spawned characters
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	TSet<const ACharacter*> charactersOnMap_;

protected:
	friend FCell;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when this actor is explicitly being destroyed
	virtual void Destroyed() override;

	// Create LevelMap on Scene and fill TMap
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void GenerateLevelMap();

	// Storage of cells and their actors
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++", meta = (DisplayName = "Grid Array"))
	TMap<FCell, const AActor*> GeneratedMap_;

	// Debug function to find nearest cell
	UFUNCTION(BlueprintImplementableEvent, Category = "C++", meta = (DevelopmentOnly))
	FCell GetNearestCell(const AActor* actor);
};