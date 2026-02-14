// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"

// Bomber
#include "Structures/BmrCell.h"

#include "BmrBombAbilityActor.generated.h"

/**
 * Is part of the UBmrBombPlaceAbility and is responsible mainly for registration and visuals:
 * - Contains the Map Component to register own location on the Generated Map, so it can be seen by other systems (like AI, other bombs, etc.).
 * - Applies mesh and material based on the instigator (player who placed the bomb).
 * - Applies collisions to block other players.
 *
 * It does NOT handle detonation timer, damage application, explosion VFXs/SFXs, etc. - all that is handled by ability, gameplay effects and cues.
 *
 * @see Access its data with UBmrBombDataAsset (Content/Bomber/DataAssets/DA_Bomb).
 */
UCLASS()
class BOMBER_API ABmrBombAbilityActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABmrBombAbilityActor();

	/** Returns the Ability System Component of the instigator who placed this bomb, is used to get attributes like fire radius. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UAbilitySystemComponent* GetInstigatorAbilitySystemComponent() const { return InstigatorAbilitySystemComponent; }

	/** Returns true when bomb is fully initialized: both bomb is initialized and added to level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsBombReady() const;

protected:
	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrMapComponent> MapComponent = nullptr;

	/** Ability System Component of the instigator who placed this bomb, is used to get attributes like fire radius. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_InstigatorAbilitySystemComponent", AdvancedDisplay, Category = "[Bomber]")
	TObjectPtr<UAbilitySystemComponent> InstigatorAbilitySystemComponent = nullptr;

	/*********************************************************************************************
	 * Detonation
	 ********************************************************************************************* */
public:
	/** Initiates the explosion: starts countdown and initializes the data (fire radius, explosion cells, etc.).
	 * Can be called on both server and clients.
	 * @param InASC - Ability System Component of the instigator who placed this bomb, is used to get attributes like fire radius. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void InitBomb(UAbilitySystemComponent* InASC);

	/** Returns cells are going to explode by this bomb. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE TSet<FBmrCell>& GetExplosionCells() const { return ExplosionCells; }

	/** Returns explosion radius from instigator, or -1 if can not be obtained. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetFireRadius() const;

	/** Show current explosion cells if the bomb type is allowed to be displayed, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (DevelopmentOnly))
	void TryDisplayExplosionCells();

protected:
	/** Is not replicated, is calculated locally on the server-only from the Fire Radius attribute of the instigator,which placed the bomb.
	 * Is not used for detonation logic, but only for AI perception and debug visuals. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TSet<FBmrCell> ExplosionCells = FBmrCell::EmptyCells;

	/** Calculates the explosion cells based on current fire radius. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void UpdateExplosionCells();

	/*********************************************************************************************
	 * Visuals: VFXs, SFXs, Mesh, Materials
	 ********************************************************************************************* */
public:
	/** Updates current mesh for this bomb actor, based on instigator type, or randomly if no instigator. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplyMesh();

	/** Updates current material for this bomb actor, based on this bomb and Player placer types. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplyMaterial();

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called on client to init bomb on clients when instigator's ASC is replicated. */
	UFUNCTION()
	void OnRep_InstigatorAbilitySystemComponent();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when this level actor is reconstructed or added on the Generated Map.
	 * Is used by Level Actors instead of the BeginPlay(). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnAddedToLevel(UBmrMapComponent* InMapComponent);

	/** Called when bomb is fully initialized: both bomb is initialized and added to level.
	 * Is first safe place to execute any logic. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnBombReady();

	/** Is used for cleaning up the bomb's data after it was removed from the level. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostRemovedFromLevel(UBmrMapComponent* InMapComponent, UObject* DestroyCauser);

	/** Is called when character leaves the bomb to update collision response. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnAnyPawnCellExit(UBmrMapComponent* PlayerMapComponent, const FBmrCell& NewCell, const FBmrCell& PreviousCell);

	/*********************************************************************************************
	 * Custom Collision Response
	 ********************************************************************************************* */
public:
	/** Returns true if this bomb was spawned for server replica of client (SROC), meaning server is processing bomb placed by client. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsServerReplicaOfClient() const;

	/** Sets actual collision response to all players for this bomb. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void InitCollisionResponseToAllPlayers();

	/** Takes your container and returns is with new specified response for player by its specified ID.
	 * @param InOutCollisionResponses Will contain requested response.
	 * @param PlayerId Player to set response.
	 * @param NewResponse New response to set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static void GetCollisionResponseToPlayerByID(FCollisionResponseContainer& InOutCollisionResponses, int32 PlayerId, ECollisionResponse NewResponse);

protected:
	/** Attempts to bind bomb to wait for instigator pawn to reach bomb cell before initializing collision.
	 * @return true if bomb was bound successfully and is now waiting for pawn to reach bomb cell. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	bool TryBindOnInstigatorReachedBombCell();

	/** Called when instigator pawn reaches the bomb cell, initializes collision. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnInstigatorPawnCellEnter(UBmrMapComponent* InMapComponent, const FBmrCell& NewCell, const FBmrCell& PreviousCell);
};