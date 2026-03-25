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
	/** Data Registry type name for difficulty rows. */
	static inline const FName DifficultyRegistryTypeName = TEXT("DR_GameDifficulty");

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

	/** Sets new difficulty tag on ASC, adds new before removing old to prevent unnecessary MGF unload/reload. */
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
	/** Returns all difficulty rows gathered from Data Registry, sorted by DifficultyLevel. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	void GetAllDifficultyRows(TArray<FBmrGameDifficultyRow>& OutRows) const;

	/** Finds difficulty row by tag, returns nullptr if not found. */
	const FBmrGameDifficultyRow* FindRowByTag(const FBmrGameDifficultyTag& Tag) const;

	/** Finds difficulty row by level, returns nullptr if not found. */
	const FBmrGameDifficultyRow* FindRowByLevel(int32 Level) const;

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

	/** Called when the level actor data is initialized and ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGeneratedMapReady(class ABmrGeneratedMap* GeneratedMap);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Config property persisting last chosen difficulty tag. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrGameDifficultyTag DifficultyTag;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Subscribes to readiness events for ASC tag binding. */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Cleans up ASC bindings. */
	virtual void OnWorldEndPlay(UWorld& InWorld) override;
};