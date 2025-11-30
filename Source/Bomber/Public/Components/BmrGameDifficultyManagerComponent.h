// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"

#include "BmrGameDifficultyManagerComponent.generated.h"

enum class EBmrGameDifficulty : uint8;

/**
 * Contains difficulty settings that are tweaked by player in Settings menu during the game.
 */
UCLASS(BlueprintType, Blueprintable, Config = "GameUserSettings", DefaultConfig)
class BOMBER_API UBmrGameDifficultyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrGameDifficultyManagerComponent();

	/** Returns this manager, is checked and wil crash if can't be obtained.*/
	static UBmrGameDifficultyManagerComponent& Get();

	/** Returns the pointer to this manager. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UBmrGameDifficultyManagerComponent* GetGameDifficultyManager(const UObject* OptionalWorldContext = nullptr);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameDifficultyChanged, int32, NewDifficultyLevel);

	/** Called when new difficulty level is set. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnGameDifficultyChanged OnGameDifficultyChanged;

	/** Returns current difficulty as enum type, e.g: EBmrGameDifficulty::Easy */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	EBmrGameDifficulty GetDifficultyType() const;

	/** Sets new game difficulty by enum type. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetDifficultyType(EBmrGameDifficulty InDifficultyType);

	/** Returns true if the game difficulty level is matched with one or more specified types. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool HasDifficulty(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EBmrActorType")) int32 DifficultiesBitmask) const;

	/** Returns current difficulty level, where:
	 * 0 - EBmrGameDifficulty::Easy; 1 - EBmrGameDifficulty::Medium; 2 - EBmrGameDifficulty::Hard
	 * Use GetDifficultyType() if is needed to obtain enum type instead of level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetDifficultyLevel() const { return ReplicatedDifficultyLevel != INDEX_NONE ? ReplicatedDifficultyLevel : DifficultyLevel; }

	/** Set new difficulty level. Higher value bigger difficulty.
	 * Where 0 is the easiest and 3 is the hardest.
	 * Use SetDifficultyType() if is needed to difficulty by enum type instead of level. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetDifficultyLevel(int32 InLevel);

	/** Applies the current difficulty by loading relevant features and unloading irrelevant ones. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void UpdateGameFeaturesByDifficulty();

protected:
	/** Applies current difficulty level to the game. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyGameDifficulty();

	/** Called on client when game difficulty level is changed. */
	UFUNCTION()
	void OnRep_ReplicatedDifficultyLevel();

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The game difficulty level, where:
	 * 0 - EBmrGameDifficulty::Easy; 1 - EBmrGameDifficulty::Medium; 2 - EBmrGameDifficulty::Hard etc.
	 * It uses integer to be able to work with Settings menu.
	 * Is config property, can be set in Settings menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 DifficultyLevel;

	/** Game difficulty of the host, which is replicated to all clients.
	 * It's separated from config property, so host's difficulty is only applied on clients, but not saved in their configs. */
	UPROPERTY(Transient, ReplicatedUsing = "OnRep_ReplicatedDifficultyLevel")
	int32 ReplicatedDifficultyLevel = INDEX_NONE;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};