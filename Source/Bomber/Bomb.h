// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "GameFramework/Actor.h"

#include "Bomb.generated.h"

UCLASS()
class BOMBER_API ABomb : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABomb();

	void InitializeBombProperties(int32* outBombN, const int32& fireN, const int32& playerID);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* mapComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	class UParticleSystem* explosionParticle;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UStaticMeshComponent* bombMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	TSet<FCell> explosionCells_;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	//Called when an instance of this class is placed (in editor) or spawned.
	void OnConstruction(const FTransform& Transform) override;

	// Called when this actor is explicitly being destroyed
	void Destroyed() override;

	/** 
	 *	Event when an actor no longer overlaps another actor and can to block collision. 
	 *	@note Components on both this and the other Actor must have bGenerateOverlapEvents set to true to generate overlap events.
	 */
	void NotifyActorEndOverlap(AActor* OtherActor) override;

	UPROPERTY(EditAnywhere, Category = "C++")
	float lifeSpan_ = 2.f;

	// Amount of character bombs at current time
	int32* characterBombN_;

	// All used bomb materials
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	TArray<class UMaterialInterface*> bombMaterials_;
};
