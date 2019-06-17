// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class UStaticMeshComponent* bombMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when this actor is explicitly being destroyed
	virtual void Destroyed() override;

	/** 
	 *	Event when an actor no longer overlaps another actor and can to block collision. 
	 *	@note Components on both this and the other Actor must have bGenerateOverlapEvents set to true to generate overlap events.
	 */
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	UPROPERTY()
	class UMapComponent* mapComponent_;

	UPROPERTY(EditAnywhere, Category = "C++")
	float lifeSpan_ = 2.f;

	// character's number of bombs (MyCharacter::powerups.fireN)
	UPROPERTY(EditAnywhere, Category = "C++", meta = (DisplayName = "Explosion Length"))
	int32 characterFireN_ = 1;

	// Amount of character bombs at current time
	int32* characterBombN_;

	// All used bomb materials
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	TArray<class UMaterialInterface*> bombMaterials_;

	//UPROPERTY(BlueprintReadOnly, Catergory = "C++")
	//TSet<struct FCell*> explosionCells;
	friend class AGeneratedMap;
};
