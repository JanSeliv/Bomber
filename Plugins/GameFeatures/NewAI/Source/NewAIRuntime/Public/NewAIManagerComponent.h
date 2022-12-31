// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "Bomber.h"
//---
#include "NewAIManagerComponent.generated.h"

/**
 * Contains common logic to manage all AI agents at once.
 */
UCLASS(Blueprintable, DisplayName = "NewAI Manager Component", ClassGroup = (Custom),
	meta = (BlueprintSpawnableComponent))
class NEWAIRUNTIME_API UNewAIManagerComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UNewAIManagerComponent();

	/** Returns the NewAI data asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class UNewAIDataAsset* GetNewAIDataAsset();

protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Disables all vanilla AI agents to override its behavior by the NewAI feature. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void DisableOriginalAI();

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** NewAI data asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		meta = (BlueprintProtected, DisplayName = "NewAI Data Asset", ShowOnlyInnerProperties))
	TObjectPtr<UNewAIDataAsset> NewAIDataAssetInternal;
};
