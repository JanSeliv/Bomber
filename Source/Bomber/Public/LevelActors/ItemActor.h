// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "GameFramework/Actor.h"

#include "ItemActor.generated.h"

/** 
 * Affects the abilities of a player during gameplay
 */
UCLASS()
class BOMBER_API AItemActor final : public AActor
{
	GENERATED_BODY()

public:
	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++")
	class UMapComponent* MapComponent;  //[C.AW]

	/** The static mesh component of this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* ItemMeshComponent;  //[C.DO]

	/** Type and its class as associated pairs  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TMap<EItemType, class UStaticMesh*> ItemTypesByMeshes;  //[M.DO]

	/**
	 * Skate: Increase the movement speed of the character.
	 * Bomb: Increase the number of bombs that can be set at one time.
	 * Fire: Increase the bomb blast radius.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	EItemType ItemType = EItemType::None;  // [AW]

	/** Sets default values for this actor's properties */
	AItemActor();

protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/**
	 * Called when a character starts to overlaps the ItemCollisionComponent component.
	 * Increases +1 to numbers of character's powerups (Skate/Bomb/Fire).
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
