// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "Structures/Cell.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "BombActor.generated.h"

#define DEFAULT_FIRE_RADIUS 1

/**
 * Describes common data for all bombs.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBombDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBombDataAsset();

	/** Returns the bomb data asset. */
	static const UBombDataAsset& Get();

	/** Get the bomb lifetime. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetLifeSpan() const { return LifeSpanInternal; }

	/** Returns the amount of bomb materials. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetBombMaterialsNum() const { return BombMaterialsInternal.Num(); }

	/** Returns the bomb material by specified index.
	 * @see UBombDataAsset::BombMaterialInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMaterialInterface* GetBombMaterial(int32 Index) const { return BombMaterialsInternal.IsValidIndex(Index) ? BombMaterialsInternal[Index] : nullptr; }

	/** Get the bomb explosion VFX. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UNiagaraSystem* GetExplosionVFX() const { return ExplosionVFXInternal; }

protected:
	/** The lifetime of a bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Life Span", ShowOnlyInnerProperties))
	float LifeSpanInternal = 2.f; //[D]

	/** All bomb materials. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Bomb Materials", ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> BombMaterialsInternal; //[D]

	/** The emitter of the bomb explosion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Explosion Particle", ShowOnlyInnerProperties))
	TObjectPtr<class UNiagaraSystem> ExplosionVFXInternal = nullptr; //[D]
};

/** Bombs are left by the character to destroy the level actors, trigger other bombs */
UCLASS()
class BOMBER_API ABombActor final : public AActor
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	ABombActor();

	/** Returns cells that bombs is going to destroy. */
	UFUNCTION(BlueprintPure, Category = "C++")
	TSet<FCell> GetExplosionCells() const;

	/** Returns radius of the blast to each side. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetExplosionRadius() const { return FireRadiusInternal; }

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
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr; //[C.AW]

	/** The radius of the blast to each side, is set by player with InitBomb on spawning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "Fire Radius"))
	int32 FireRadiusInternal = INDEX_NONE; //[N]

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

	/** Destroy bomb and burst explosion cells, calls multicast event.*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected, DefaultToSelf = "DestroyedActor"))
	void DetonateBomb();

	/** Destroy bomb and burst explosion cells.
	  * Calls destroying request of all actors by cells in explosion cells array.*/
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "C++", meta = (BlueprintProtected))
	void MulticastDetonateBomb();

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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetCollisionResponseToPlayer(FCollisionResponseContainer& OutCollisionResponses, int32 CharacterID, ECollisionResponse NewResponse) const;

	/** Gets the response for all players.
	  * @param OutCollisionResponses Returns requested responses. 
	  * @param NewResponse New response to set. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
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
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetCollisionResponseToPlayers(FCollisionResponseContainer& OutCollisionResponses, int32 Bitmask, ECollisionResponse BitOnResponse, ECollisionResponse BitOffResponse) const;

	/** Returns all players overlapping with this bomb. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetOverlappingPlayers(TArray<AActor*>& OutPlayers) const;

	/** Updates current material for this bomb actor. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyMaterial();

	/** Is called on client to respond on changes in material of the bomb. */
	UFUNCTION()
	void OnRep_BombMaterial();
};
