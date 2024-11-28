// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"
//---
#include "NewAIBaseSubsystem.generated.h"

class UNewAIDataAsset;

enum class ECurrentGameState : uint8;

/**
 * Provides access to the most important data like Data Asset.
 */
UCLASS(BlueprintType, Blueprintable, Config = "NewAI", DefaultConfig)
class NEWAI_API UNewAIBaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNewAIBaseSubsystem& Get(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Data Asset
	 ********************************************************************************************* */
public:
	/** Returns the NewAI data asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const UNewAIDataAsset* GetNewAIDataAsset() const;

protected:
	/** NewAI data asset */
	UPROPERTY(Config, VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "NewAI Data Asset"))
	TSoftObjectPtr<const UNewAIDataAsset> NewAIDataAssetInternal;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when the game starts. */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Clears all transient data contained in this subsystem. */
	virtual void Deinitialize() override;

	/** Disables or enables all vanilla AI agents to override its behavior by the NewAI feature. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void HandleLegacyAI();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Called when new difficulty level is set. */
	UFUNCTION(BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnNewAIDifficultyChanged(int32 NewDifficultyLevel);
};
