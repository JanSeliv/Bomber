// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BmrActorUtilsLibrary.generated.h"

class UBmrMapComponent;

/**
 * The static function library for obtaining various project-related actors.
 * @TODO JanSeliv RbZPYyEY move actors-related functions and replace arguments from Map Components to Actors. Until then, call MapComponent->GetOwner() to obtain Level Actor
 */
UCLASS()
class BOMBER_API UBmrActorUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Level Actors
	 * It's alternative to UBmrCellUtilsLibrary, but for obtaining level actors.
	 ********************************************************************************************* */
public:
	/** Level Actors getter.
	 * @param OutBitmaskedComponents Will contains map components of owners having the specified types.
	 * @param ActorsTypesBitmask EBmrActorType bitmask of actors types.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static void GetLevelActors(
	    TSet<UBmrMapComponent*>& OutBitmaskedComponents,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Returns level actors that are located on the specified cells.
	 * @param OutMapComponents Will contains map components of owners located on the specified cells.
	 * @param InCells Cells to check.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static void GetLevelActorsOnCells(TSet<UBmrMapComponent*>& OutMapComponents, const TSet<struct FBmrCell>& InCells);

	/** Returns the index comparing to actors of its type on the Generated Map.
	 * For instance, if the cell is a player, given cell is third player on the level, it will return 2. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static int32 GetIndexByLevelActor(const UBmrMapComponent* InMapComponent);

	/** Returns the level actor by its index: order of creation in the level.
	 * @param Index Index of the actor in the level.
	 * @param ActorsTypesBitmask EBmrActorType bitmask of actors types.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static UBmrMapComponent* GetLevelActorByIndex(int32 Index,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);

	/** Takes level actors and returns only matching with specified actor types.
	 * Could be useful to extract only needed actors.
	 * @param InActors Actors to filter.
	 * @param ActorsTypesBitmask Bitmask of actors types to filter.
	 * @return actors of specified types. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static TSet<UBmrMapComponent*> FilterLevelActors(
	    const TSet<UBmrMapComponent*>& InActors,
	    UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 ActorsTypesBitmask);
};