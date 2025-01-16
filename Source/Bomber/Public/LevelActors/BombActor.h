// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "BombActor.generated.h"

#define DEFAULT_LIFESPAN -1.f

enum class ELevelType : uint8;

/**
 * Bombs are put by the character to destroy the level actors, trigger other bombs.
 * @see Access its data with UBombDataAsset (Content/Bomber/DataAssets/DA_Bomb).
 */
UCLASS()
class BOMBER_API ABombActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABombActor();

	/** Preinitialize a bomb actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructBombActor();

	/** Returns the type of the bomb. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ELevelType GetBombType() const;

protected:
	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr;

	/*********************************************************************************************
	 * Detonation
	 ********************************************************************************************* */
public:
	/** Sets the defaults of the bomb.
	 * @param BombPlacer - the player who placed the bomb. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void InitBomb(const class APlayerCharacter* BombPlacer = nullptr);

	/** Returns cells that bombs is going to destroy. */
	UFUNCTION(BlueprintPure, Category = "C++")
	TSet<struct FCell> GetExplosionCells() const;

	/** Returns radius of the blast to each side.
	 * It might be overriden by the cheat manager. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetFireRadius() const { return FireRadiusInternal; }

	/** Returns the character who placed the bomb. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE class APlayerCharacter* GetBombPlacer() const { return BombPlacerInternal; }

	/** Show current explosion cells if the bomb type is allowed to be displayed, is not available in shipping build. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void TryDisplayExplosionCells();

protected:
	/** The radius of the blast to each side, is during initialization: from the player, cheat manager or 1 as a default. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Replicated, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Fire Radius"))
	int32 FireRadiusInternal = 0;

	/** The character who placed the bomb, is set by InitBomb on spawning.
	 * Is used to track who spawned the bomb, e.g: to record the score. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_BombPlacer", AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Bomb Placer"))
	TObjectPtr<const class APlayerCharacter> BombPlacerInternal = nullptr;

	/** Is called on client to update current bomb placer. */
	UFUNCTION()
	void OnRep_BombPlacer();

	/** Destroy bomb and burst explosion cells, calls multicast event.*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected, DefaultToSelf = "DestroyedActor"))
	void DetonateBomb();

	/** Destroy bomb and burst explosion cells.
	 * Calls destroying request of all actors by cells in explosion cells array.*/
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "C++", meta = (BlueprintProtected))
	void MulticastDetonateBomb(const TArray<struct FCell>& ExplosionCells);

	/*********************************************************************************************
	 * Cue Visuals: VFXs, SFXs, Materials
	 ********************************************************************************************* */
public:
	/** Spawns VFXs and SFXs, is allowed to call both on server and clients. */
	UFUNCTION(Blueprintable, Category = "C++")
	void PlayExplosionsCue(const TArray<struct FCell>& ExplosionCells);

	/** Updates current material for this bomb actor, based on this bomb and Player placer types. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ApplyMaterial();

protected:
	/** All currently playing VFXs. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Spawned VFXs"))
	TArray<TObjectPtr<class UNiagaraComponent>> SpawnedVFXsInternal;

	/** The duration of the bomb VFX. */
	FTimerHandle VFXDurationExpiredTimerHandle;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Set the lifespan of this actor. When it expires the object will be destroyed.
	 * @param InLifespan overriden with a default value, time will be got from the data asset. */
	virtual void SetLifeSpan(float InLifespan = DEFAULT_LIFESPAN) override;

	/** Called when the lifespan of an actor expires (if he has one). */
	virtual void LifeSpanExpired() override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called on a bomb actor construction, could be called multiple times.
	 * Could be listened by binding to UMapComponent::OnOwnerWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructBombActor() -> UMapComponent::ConstructOwnerActor() -> ThisClass::OnConstructionBombActor().
	 * @warning Do not call directly, use ThisClass::ConstructBombActor() instead. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnConstructionBombActor();

	/** Triggers when character end to overlaps with this bomb.
	 * Sets the collision preset to block all dynamics. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Called when owned map component is destroyed on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPreRemovedFromLevel(UMapComponent* MapComponent, UObject* DestroyCauser);

	/*********************************************************************************************
	 * Custom Collision Response
	 ********************************************************************************************* */
public:
	/** Sets actual collision response to all players for this bomb. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateCollisionResponseToAllPlayers();

	/** Takes your container and returns is with new specified response for player by its specified ID.
	 * @param InOutCollisionResponses Will contain requested response.
	 * @param CharacterID Player to set response.
	 * @param NewResponse New response to set. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	static void MakeCollisionResponseToPlayerByID(FCollisionResponseContainer& InOutCollisionResponses, int32 CharacterID, ECollisionResponse NewResponse);

	/** Takes your container and returns new specified response for all players.
	  * @param InOutCollisionResponses Will contain requested responses.
	  * @param NewResponse New response to set. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	static void MakeCollisionResponseToAllPlayers(FCollisionResponseContainer& InOutCollisionResponses, ECollisionResponse NewResponse);

	/** Takes your container and returns new specified response for those players who match their ID in specified bitmask.
	  * @param InOutCollisionResponses Will contain requested responses.
	  * @param Bitmask Each bit represents the character ID.
	  * @param BitOnResponse Applies response for toggles bits.
	  * @param BitOffResponse Applies response for clear bits.
	  * Set 'ECollisionResponse::ECR_MAX' to avoid changing response for toggled or clear bits.
	  * E.g: Bitmask = 11, BitOnResponse = ECR_Block, BitOffResponse = ECR_MAX:
	  * specified '11' in binary is '1 0 1 1',
	  * so characters with IDs '0', '1' and '3' will apply 'ECR_Block' response,
	  * player with Character ID '2' won't change its response since it's specified as 'ECR_MAX'. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	static void MakeCollisionResponseToPlayersInBitmask(FCollisionResponseContainer& InOutCollisionResponses, int32 Bitmask, ECollisionResponse BitOnResponse, ECollisionResponse BitOffResponse);

	/** Returns all players overlapping with this bomb. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (BlueprintProtected))
	void GetOverlappingPlayers(TArray<AActor*>& OutPlayers) const;
};