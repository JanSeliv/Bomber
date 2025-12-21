// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DeveloperSettings.h"

#include "BmrDataAssetsContainer.generated.h"

enum class EBmrActorType : uint8;
enum class EBmrLevelType : uint8;

class UBmrLevelActorDataAsset;

/**
 * Contains all core data of the game.
 * Is set up in 'Project Settings' -> 'Game' -> 'Data Assets Container'.
 * The changes are saved in 'DefaultDataAssets.ini' file.
 */
UCLASS(Config = "DataAssets", DefaultConfig, DisplayName = "Bomber Data Assets Container")
class BOMBER_API UBmrDataAssetsContainer final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Returns the data assets container. */
	static const UBmrDataAssetsContainer& Get() { return *GetDefault<ThisClass>(); }

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** Returns the Levels Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const class UBmrGeneratedMapDataAsset* GetGeneratedMapDataAsset();

	/** Returns the UI Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "Get UI Data Asset"))
	static const class UBmrUIDataAsset* GetUIDataAsset();

	/** Returns the AI Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DisplayName = "Get AI Data Asset"))
	static const class UBmrAIDataAsset* GetAIDataAsset();

	/** Returns the Player Input Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const class UBmrPlayerInputDataAsset* GetPlayerInputDataAsset();

	/** Returns the Sounds Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (Keywords = "Sound"))
	static const class UBmrSoundsDataAsset* GetSoundsDataAsset();

	/** Returns the Game State Data Asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const class UBmrGameStateDataAsset* GetGameStateDataAsset();

	/*********************************************************************************************
	 * Getters of Level Actor's data assets
	 ********************************************************************************************* */
public:
	/** Best suits for blueprints to get the data asset by its class since converts the result to the specified class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (DeterminesOutputType = "DataAssetClass", BlueprintAutocast, Keywords = "Bomb,Box,Powerup,Wall,Pawn,Player,Character"))
	static const UBmrLevelActorDataAsset* GetLevelActorDataAsset(
	    UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UBmrLevelActorDataAsset> DataAssetClass);

	/** Returns the data asset by its class, if not found then crash. */
	template <typename T = UBmrLevelActorDataAsset>
	static const FORCEINLINE T& GetLevelActorDataAssetChecked()
	{
		static_assert(TIsDerivedFrom<T, UBmrLevelActorDataAsset>::IsDerived, "T must be a subclass of UBmrLevelActorDataAsset");
		return *CastChecked<T>(GetLevelActorDataAsset(T::StaticClass()));
	}

	/** Iterates ActorsDataAssets array and returns the found Data Asset by specified actor class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const UBmrLevelActorDataAsset* GetDataAssetByActorClass(const TSubclassOf<class AActor> ActorClass);

	/** Iterate ActorsDataAssets array and returns the found Data Assets of level actors by specified types. */
	static void GetDataAssetsByActorTypes(TArray<const UBmrLevelActorDataAsset*>& OutDataAssets, int32 ActorsTypesBitmask);

	/** Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static const UBmrLevelActorDataAsset* GetDataAssetByActorType(EBmrActorType ActorType);

	/** Iterate ActorsDataAssets array and returns the found actor class by specified actor type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static UClass* GetActorClassByType(EBmrActorType ActorType);

	/*********************************************************************************************
	 * Data Assets
	 ********************************************************************************************* */
protected:
	/** Contains properties to setup the generated level, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<const class UBmrGeneratedMapDataAsset> GeneratedMapDataAsset;

	/** Contains properties to setup UI, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<const class UBmrUIDataAsset> UIDataAsset;

	/** AI data, is config property, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<const class UBmrAIDataAsset> AIDataAsset;

	/** Player Input data, is config property, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<const class UBmrPlayerInputDataAsset> PlayerInputDataAsset;

	/** Sounds data, is config property, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<const class UBmrSoundsDataAsset> SoundsDataAsset;

	/** The data of the game match, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TSoftObjectPtr<const class UBmrGameStateDataAsset> GameStateDataAsset;

	/** Actor type and its associated class, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<TSoftObjectPtr<const UBmrLevelActorDataAsset>> ActorsDataAssets;
};
