// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"

#include "GameDifficultyManagerComponent.generated.h"

enum class EGameDifficulty : uint8;

/**
 * Contains difficulty settings that are tweaked by player in Settings menu during the game.
 */
UCLASS(BlueprintType, Blueprintable, Config = "GameUserSettings", DefaultConfig)
class BOMBER_API UGameDifficultyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UGameDifficultyManagerComponent();

	/** Returns this manager, is checked and wil crash if can't be obtained.*/
	static UGameDifficultyManagerComponent& Get();

	/** Returns the pointer to this manager. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UGameDifficultyManagerComponent* GetGameDifficultyManager(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameDifficultyChanged, int32, NewDifficultyLevel);

	/** Called when new difficulty level is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnGameDifficultyChanged OnGameDifficultyChanged;

	/** Returns current difficulty as enum type, e.g: EGameDifficulty::Easy */
	UFUNCTION(BlueprintPure, Category = "C++")
	EGameDifficulty GetDifficultyType() const;

	/** Sets new game difficulty by enum type. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetDifficultyType(EGameDifficulty InDifficultyType);

	/** Returns true if the game difficulty level is matched with one or more specified types. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool HasDifficulty(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 DifficultiesBitmask) const;

	/** Returns current difficulty level, where:
	 * 0 - EGameDifficulty::Easy; 1 - EGameDifficulty::Medium; 2 - EGameDifficulty::Hard
	 * Use GetDifficultyType() if is needed to obtain enum type instead of level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetDifficultyLevel() const { return ReplicatedDifficultyLevelInternal != INDEX_NONE ? ReplicatedDifficultyLevelInternal : DifficultyLevelInternal; }

	/** Set new difficulty level. Higher value bigger difficulty.
	 * Where 0 is the easiest and 3 is the hardest.
	 * Use SetDifficultyType() if is needed to difficulty by enum type instead of level. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetDifficultyLevel(int32 InLevel);

	/** Applies the current difficulty by loading relevant features and unloading irrelevant ones. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateGameFeaturesByDifficulty();

protected:
	/** Applies current difficulty level to the game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyGameDifficulty();

	/** Called on client when game difficulty level is changed. */
	UFUNCTION()
	void OnRep_ReplicatedDifficultyLevel();

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The game difficulty level, where:
	 * 0 - EGameDifficulty::Easy; 1 - EGameDifficulty::Medium; 2 - EGameDifficulty::Hard etc.
	 * It uses integer to be able to work with Settings menu.
	 * Is config property, can be set in Settings menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Difficulty Level"))
	int32 DifficultyLevelInternal;

	/** Game difficulty of the host, which is replicated to all clients.
	 * It's separated from config property, so host's difficulty is only applied on clients, but not saved in their configs. */
	UPROPERTY(Transient, ReplicatedUsing = "OnRep_ReplicatedDifficultyLevel")
	int32 ReplicatedDifficultyLevelInternal = INDEX_NONE;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};