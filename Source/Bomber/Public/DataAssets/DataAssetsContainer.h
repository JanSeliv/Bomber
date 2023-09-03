// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DeveloperSettings.h"
//---
#include "DataAssetsContainer.generated.h"

enum class EActorType : uint8;
enum class ELevelType : uint8;

class ULevelActorDataAsset;

/**
 * Contains all core data of the game.
 * Is set up in 'Project Settings' -> 'Game' -> 'Data Assets Container'.
 * The changes are saved in 'DefaultDataAssets.ini' file.
 */
UCLASS(Config = "DataAssets", DefaultConfig, DisplayName = "Data Assets Container")
class BOMBER_API UDataAssetsContainer final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Returns the data assets container. */
	static const UDataAssetsContainer& Get() { return *GetDefault<ThisClass>(); }

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** Returns the Levels Data Asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class UGeneratedMapDataAsset* GetGeneratedMapDataAsset();

	/** Returns the UI Data Asset. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get UI Data Asset"))
	static const class UUIDataAsset* GetUIDataAsset();

	/** Returns the AI Data Asset. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get AI Data Asset"))
	static const class UAIDataAsset* GetAIDataAsset();

	/** Returns the Player Input Data Asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class UPlayerInputDataAsset* GetPlayerInputDataAsset();

	/** Returns the Sounds Data Asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class USoundsDataAsset* GetSoundsDataAsset();

	/** Returns the Game State Data Asset. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class UGameStateDataAsset* GetGameStateDataAsset();

	/*********************************************************************************************
	 * Getters of Level Actor's data assets
	 ********************************************************************************************* */
public:
	/** Best suits for blueprints to get the data asset by its class since converts the result to the specified class. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DeterminesOutputType = "DataAssetClass", BlueprintAutocast, Keywords = "Bomb,Box,Item,Player,Character,Wall"))
	static const ULevelActorDataAsset* GetLevelActorDataAsset(
		UPARAM(meta=(AllowAbstract="false")) TSubclassOf<ULevelActorDataAsset> DataAssetClass);

	/** Returns the data asset by its class, if not found then crash. */
	template <typename T = ULevelActorDataAsset>
	static const FORCEINLINE T& GetLevelActorDataAssetChecked()
	{
		static_assert(TIsDerivedFrom<T, ULevelActorDataAsset>::IsDerived, TEXT("T must be a subclass of ULevelActorDataAsset"));
		return *CastChecked<T>(GetLevelActorDataAsset(T::StaticClass()));
	}

	/** Iterates ActorsDataAssets array and returns the found Data Asset by specified actor class. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const ULevelActorDataAsset* GetDataAssetByActorClass(const TSubclassOf<class AActor> ActorClass);

	/** Iterate ActorsDataAssets array and returns the found Data Assets of level actors by specified types. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static void GetDataAssetsByActorTypes(
		TArray<ULevelActorDataAsset*>& OutDataAssets,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

	/** Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const ULevelActorDataAsset* GetDataAssetByActorType(EActorType ActorType);

	/** Iterate ActorsDataAssets array and returns the found actor class by specified actor type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static UClass* GetActorClassByType(EActorType ActorType);

	/*********************************************************************************************
	 * Data Assets
	 ********************************************************************************************* */
protected:
	/** Contains properties to setup the generated level, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Levels Data Asset", ShowOnlyInnerProperties))
	TSoftObjectPtr<class UGeneratedMapDataAsset> GeneratedMapDataAssetInternal;

	/** Contains properties to setup UI, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "UI Data Asset", ShowOnlyInnerProperties))
	TSoftObjectPtr<class UUIDataAsset> UIDataAssetInternal;

	/** AI data, is config property, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "AI Data Asset", ShowOnlyInnerProperties))
	TSoftObjectPtr<class UAIDataAsset> AIDataAssetInternal;

	/** Player Input data, is config property, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Player Input Data Asset", ShowOnlyInnerProperties))
	TSoftObjectPtr<class UPlayerInputDataAsset> PlayerInputDataAssetInternal;

	/** Sounds data, is config property, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Sounds Data Asset", ShowOnlyInnerProperties))
	TSoftObjectPtr<class USoundsDataAsset> SoundsDataAssetInternal;

	/** The data of the game match, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Game State Data Asset", ShowOnlyInnerProperties))
	TSoftObjectPtr<class UGameStateDataAsset> GameStateDataAssetInternal;

	/** Actor type and its associated class, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Actors Data Assets", ShowOnlyInnerProperties))
	TArray<TSoftObjectPtr<ULevelActorDataAsset>> ActorsDataAssetsInternal;
};
