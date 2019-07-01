// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "GameFramework/Actor.h"

#include "Bomb.generated.h"

UCLASS()
class BOMBER_API ABomb final : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABomb();

	void InitializeBombProperties(int32* OutBombN, const int32& FireN, const int32& CharacterID);

	/** The Map Component manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	class UParticleSystem* ExplosionParticle;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UStaticMeshComponent* BombMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	TSet<FCell> ExplosionCells_;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() final;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) final;

	/** Event triggered when the actor has been explicitly destroyed */
	UFUNCTION()
	void OnBombDestroyed(AActor* DestroyedActor);

	/** 
	 *	Event when an actor no longer overlaps another actor and can to block collision. 
	 *	@note Components on both this and the other Actor must have bGenerateOverlapEvents set to true to generate overlap events.
	 */
	virtual void NotifyActorEndOverlap(AActor* OtherActor) final;

	UPROPERTY(EditAnywhere, Category = "C++")
	float LifeSpan_ = 2.f;

	UPROPERTY(EditAnywhere, Category = "C++")
	int32 ExplosionLength = 1;

	// Amount of character bombs at current time
	int32* CharacterBombN_;

	// All used bomb materials
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	TArray<class UMaterialInterface*> BombMaterials_;
};
