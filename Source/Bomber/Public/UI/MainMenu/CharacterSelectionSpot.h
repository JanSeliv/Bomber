// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/Actor.h"
#include "CharacterSelectionSpot.generated.h"

/**
 * Represents a spot where a character can be selected in the Main Menu.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API ACharacterSelectionSpot : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	ACharacterSelectionSpot();

	/*********************************************************************************************
     * Protected properties
     ********************************************************************************************* */
protected:
	/** Linked actor that contains representing mesh to be change the skin and so on. Points to actor on the level that is used by cinematic. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Mesh Actor"))
	TObjectPtr<class AMySkeletalMeshActor> CinematicMeshActorInternal = nullptr;

	/** Is current value of last chosen skin index. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Skin Index"))
	int32 SkinIndexInternal = 0;
};
