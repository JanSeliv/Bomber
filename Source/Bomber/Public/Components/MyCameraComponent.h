// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Camera/CameraComponent.h"
#include "Bomber.h"
//---
#include "MyCameraComponent.generated.h"

/**
 * The main camera viewpoint of the game.
 */
UCLASS()
class BOMBER_API UMyCameraComponent final : public UCameraComponent
{
	GENERATED_BODY()

public:
	/** The minimal camera height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float MinHeight = 1500.f; //[N]

	/** Sets default values for this actor's properties. */
	UMyCameraComponent();

	/** Set the maximum possible height. Called on construct and game start*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateMaxHeight();

	/**
	 * Set the location between players.
	 * @param DeltaTime Optional parameter, lerp if specified
	 * @return true for successful update
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	bool UpdateLocation(float DeltaTime = 0.f);

protected:
	/** The maximal camera height. Is set dynamically by UMyCameraComponent::UpdateMaxHeights(). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Max Height"))
	float MaxHeightInternal; //[G]

	/** If UMyCameraComponent::StartLocation is true, then should forced moving to the start position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Force Move To Start"))
	bool bForceStartInternal; //[N]

	/** The absolute center position between players. The camera starts game from that position and returns to it on endgame. Is not visible in editor, is set on Begin Play*/
	UPROPERTY(BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Start Location"))
	FVector StartLocationInternal; //[N]

	/** Called every frame. */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
