﻿// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "Structures/Cell.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "BombActor.generated.h"

/**
 * Describes common data for all bombs.
 */
UCLASS(Blueprintable, BlueprintType)
class UBombDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBombDataAsset();

	/** Returns the bomb data asset. */
	static const UBombDataAsset& Get();

	/** All bomb materials. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> BombMaterials; //[D]

	/** The emitter of the bomb explosion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UNiagaraSystem> ExplosionParticle = nullptr; //[D]

	/** Get the bomb lifetime. */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (ShowOnlyInnerProperties))
	FORCEINLINE float GetLifeSpan() const { return LifeSpanInternal; }

protected:
	/** The lifetime of a bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Life Span", ShowOnlyInnerProperties))
	float LifeSpanInternal = 2.f; //[D]
};

/** Bombs are left by the character to destroy the level actors, trigger other bombs */
UCLASS()
class ABombActor final : public AActor
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	ABombActor();

	/** Returns explosion cells (by copy to avoid changes). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE TArray<FCell>& GetExplosionCells() const { return ExplosionCellsInternal; }

	/**
	 * Sets the defaults of the bomb
	 * @param InFireRadius Setting explosion length of this bomb
	 * @param CharacterID Setting a mesh material of bomb by the character ID
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void InitBomb(int32 InFireRadius = 1, int32 CharacterID = -1);

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal; //[C.AW]

	/** The bomb blast path */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "Explosion Cells"))
	TArray<FCell> ExplosionCellsInternal;

	/** The radius of the blast to each side. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "Fire Radius"))
	int32 FireRadiusInternal = 1; //[N]

	/** Current material of this bomb, is different for each player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_BombMaterial", Category = "C++", meta = (BlueprintProtected, DisplayName = "Bomb Material"))
	TObjectPtr<class UMaterialInterface> BombMaterialInternal = nullptr; //[G]

	/* ---------------------------------------------------
 	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Set the lifespan of this actor. When it expires the object will be destroyed.
	 * @param InLifespan overriden with a default value, time will be got from the data asset. */
	virtual void SetLifeSpan(float InLifespan = INDEX_NONE) override;

	/** Called when the lifespan of an actor expires (if he has one). */
	virtual void LifeSpanExpired() override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/** Destroy bomb and burst explosion cells.
	  * Calls destroying request of all actors by cells in explosion cells array.*/
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "C++", meta = (BlueprintProtected, DefaultToSelf = "DestroyedActor"))
	void MulticastDetonateBomb(AActor* DestroyedActor = nullptr);

	/**
	 * Triggers when character end to overlaps with this bomb.
	 * Sets the collision preset to block all dynamics.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Listen by dragged bombs to handle game resetting. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Gets the response for specified player.
	 * @param OutCollisionResponses Returns requested response. 
	 * @param CharacterID Player to set response.
	 * @param NewResponse New response to set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetCollisionResponseToPlayer(FCollisionResponseContainer& OutCollisionResponses, int32 CharacterID, ECollisionResponse NewResponse) const;

	/** Gets the response for all players.
	  * @param OutCollisionResponses Returns requested responses. 
	  * @param NewResponse New response to set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetCollisionResponseToAllPlayers(FCollisionResponseContainer& OutCollisionResponses, ECollisionResponse NewResponse) const;

	/** Gets the response for players by specified bitmask.
	  * @param OutCollisionResponses Returns requested responses. 
	  * @param Bitmask Each bit represents the character ID.
	  * @param BitOnResponse Applies response for toggles bits.
	  * @param BitOffResponse Applies response for clear bits.
	  * Set 'ECollisionResponse::ECR_MAX' to avoid changing response for toggled or clear bits.
	  * E.g: Bitmask = 11, BitOnResponse = ECR_Block, BitOffResponse = ECR_MAX:
	  * specified '11' in binary is '1 0 1 1',
	  * so characters with IDs '0', '1' and '3' will apply 'ECR_Block' response,
	  * player with Character ID '2' won't change its response since it's specified as 'ECR_MAX'. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetCollisionResponseToPlayers(FCollisionResponseContainer& OutCollisionResponses, int32 Bitmask, ECollisionResponse BitOnResponse, ECollisionResponse BitOffResponse) const;
	
	/** Returns all players overlapping with this bomb. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetOverlappingPlayers(TArray<AActor*>& OutPlayers) const;

	/** Updates current material for this bomb actor. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyMaterial();

	/** Is called on client to respond on changes in material of the bomb. */
	UFUNCTION()
	void OnRep_BombMaterial();
};
