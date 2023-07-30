// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/MySkeletalMeshComponent.h"
#include "CharacterSelectionSpot.generated.h"

/**
 * Represents a spot where a character can be selected in the Main Menu.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API ACharacterSelectionSpot final : public AMySkeletalMeshActor
{
	GENERATED_BODY()

public:
	/** Blends camera to this spot. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCameraViewOnSpot(bool bBlend);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Linked camera actor to set the view that is also used by cinematics. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Camera Actor"))
	TObjectPtr<class ACameraActor> CameraActorInternal = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Overridable native event for when play begins for this actor. */
	virtual void BeginPlay() override;

	/** Sets camera view to this spot if current level type is equal to the spot's player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TrySetCameraViewByDefault();
};
