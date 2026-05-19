// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/WorldSubsystem.h"

// Bomber
#include "Structures/BmrGameDifficultyTag.h"

#include "BmrGameDifficultySubsystem.generated.h"

/**
 * Manages game difficulty as gameplay tags on ASC.
 */
UCLASS(BlueprintType, Blueprintable, Config = "GameUserSettings", DefaultConfig)
class BOMBER_API UBmrGameDifficultySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this subsystem, is checked and will crash if can't be obtained. */
	static UBmrGameDifficultySubsystem& Get(const UObject* WorldContextObject = nullptr);

	/** Returns the pointer to this subsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "WorldContextObject"))
	static UBmrGameDifficultySubsystem* GetGameDifficultySubsystem(const UObject* WorldContextObject = nullptr);

	/*********************************************************************************************
	 * Difficulty Tag API
	 ********************************************************************************************* */
public:
	/** Returns current difficulty tag from ASC, falls back to config if ASC unavailable. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FBmrGameDifficultyTag GetGameDifficultyTag() const;

	/** Sets new difficulty tag on ASC, adds new before removing old to prevent unnecessary GFP unload/reload. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetGameDifficultyTag(FBmrGameDifficultyTag NewTag);

	/** Returns current difficulty level integer from Data Registry row lookup. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetDifficultyLevel() const;

	/** Sets difficulty by integer level, converts to tag via Data Registry row lookup. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetDifficultyLevel(int32 InLevel);

	/** Populates combobox display names from all Data Registry difficulty rows sorted by DifficultyLevel. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void GetAllDifficultyDisplayTexts(TArray<FText>& OutMembers) const;

	/*********************************************************************************************
	 * Data Registry Row Lookup
	 ********************************************************************************************* */
public:
	/** Finds difficulty row by tag, returns nullptr if not found. */
	static const struct FBmrGameDifficultyRow* FindRowByTag(const FBmrGameDifficultyTag& Tag);

	/** Finds difficulty row by level, returns nullptr if not found. */
	static const FBmrGameDifficultyRow* FindRowByLevel(int32 Level);

	/*********************************************************************************************
	 * Event Broadcasting
	 ********************************************************************************************* */
protected:
	/** Broadcasts Event::Difficulty_Changed with current difficulty tag in InstigatorTags. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void BroadcastDifficultyChanged();

	/** Subscribes to ASC generic tag event, filters for Difficulty.* tags. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void BindOnDifficultyTagChanged();

	/** Called when the world ASC becomes available, subscribes to ASC difficulty tag events. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnWorldASCReady(const struct FGameplayEventData& Payload);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Config property persisting last chosen difficulty tag. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrGameDifficultyTag DifficultyTag = FBmrGameDifficultyTag::None;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when world begins play. */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Called when world ends play. */
	virtual void OnWorldEndPlay(UWorld& InWorld) override;
};