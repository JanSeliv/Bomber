// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "LevelActorsUtilsLibrary.generated.h"

class UMapComponent;
struct FCell;

/**
 * The static function library for Level Actors on the Generated Map.
 * It's alternative to UCellsUtilsLibrary, but for actors.
 * It works with UMapComponent since it's owned by each level actor.
 * To obtain Level Actor, call MapComponent-.GetOwner()
 */
UCLASS()
class BOMBER_API ULevelActorsUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Level Actors getter.
	 * @param OutBitmaskedComponents Will contains map components of owners having the specified types.
	 * @param ActorsTypesBitmask EActorType bitmask of actors types.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	static void GetLevelActors(
		TSet<UMapComponent*>& OutBitmaskedComponents,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Returns level actors that are located on the specified cells.
	 * @param OutMapComponents Will contains map components of owners located on the specified cells.
	 * @param InCells Cells to check.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static void GetLevelActorsOnCells(TSet<UMapComponent*>& OutMapComponents, const TSet<FCell>& InCells);

	/** Returns the index comparing to actors of its type on the Generated Map.
	 * For instance, if the cell is a player, given cell is third player on the level, it will return 2. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetIndexByLevelActor(const UMapComponent* InMapComponent);

	/** Returns the level actor by its index: order of creation in the level.
	 * @param Index Index of the actor in the level.
	 * @param ActorsTypesBitmask EActorType bitmask of actors types.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	static UMapComponent* GetLevelActorByIndex(int32 Index,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);
};