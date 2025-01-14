// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "BoxActor.generated.h"

/**
 * Boxes on destruction with some chances spawns an item.
 * @see Access its data with UBoxDataAsset (Content/Bomber/DataAssets/DA_Box).
 */
UCLASS()
class BOMBER_API ABoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABoxActor();

	/** Initialize a box actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructBoxActor();

	/** Spawn item with a chance. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TrySpawnItem();

protected:
	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called on a box actor construction, could be called multiple times.
     * Could be listened by binding to UMapComponent::OnOwnerWantsReconstruct delegate.
     * See the call stack below for more details:
    * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructBoxActor() -> UMapComponent::ConstructOwnerActor() -> ThisClass::OnConstructionBoxActor().
     * @warning Do not call directly, use ThisClass::ConstructBoxActor() instead. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnConstructionBoxActor();

	/** Called when owned map component is destroyed on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnDeactivatedMapComponent(UMapComponent* MapComponent, UObject* DestroyCauser);
};