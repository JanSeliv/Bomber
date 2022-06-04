// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "Bomber.h"
//---
#include "DataAssetsContainer.generated.h"

/**
 * Contains all core data.
 */
UCLASS()
class BOMBER_API UDataAssetsContainer : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the data assets container. */
	static const UDataAssetsContainer& Get();

	/** Returns the Levels Data Asset*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static const FORCEINLINE class UGeneratedMapDataAsset* GetLevelsDataAsset() { return Get().LevelsDataAssetInternal; }

	/** Returns the UI Data Asset*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get UI Data Asset"))
	static const FORCEINLINE class UUIDataAsset* GetUIDataAsset() { return Get().UIDataAssetInternal; }

	/** Returns the settings data.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static const FORCEINLINE class USettingsDataAsset* GetSettingsDataAsset() { return Get().SettingsDataAssetInternal; }

	/** Returns the AI data.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get AI Data Asset"))
	static const FORCEINLINE class UAIDataAsset* GetAIDataAsset() { return Get().AIDataAssetInternal; }

	/** Returns the Player Input data.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static const FORCEINLINE class UPlayerInputDataAsset* GetPlayerInputDataAsset() { return Get().PlayerInputDataAssetInternal; }

	/** Returns the Sounds data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static const FORCEINLINE class USoundsDataAsset* GetSoundsDataAsset() { return Get().SoundsDataAssetInternal; }

	/** Iterate ActorsDataAssets array and returns the found Level Actor class by specified data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorClass"))
	static const class ULevelActorDataAsset* GetDataAssetByActorClass(const TSubclassOf<AActor>& ActorClass);

	/** Iterate ActorsDataAssets array and returns the found Data Assets of level actors by specified types. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static void GetDataAssetsByActorTypes(
		TArray<class ULevelActorDataAsset*>& OutDataAssets,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static const class ULevelActorDataAsset* GetDataAssetByActorType(EActorType ActorType);

	/** Iterate ActorsDataAssets array and returns the found actor class by specified actor type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static UClass* GetActorClassByType(EActorType ActorType);

protected:
	/** Contains properties to setup the generated level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Levels Data Asset"))
	TObjectPtr<class UGeneratedMapDataAsset> LevelsDataAssetInternal = nullptr; //[B]

	/** Settings data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Data Asset"))
	TObjectPtr<class USettingsDataAsset> SettingsDataAssetInternal = nullptr; //[B]

	/** Contains properties to setup UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "UI Data Asset"))
	TObjectPtr<class UUIDataAsset> UIDataAssetInternal = nullptr; //[B]

	/** AI data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "AI Data Asset"))
	TObjectPtr<class UAIDataAsset> AIDataAssetInternal = nullptr; //[B]

	/** Player Input data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Input Data Asset"))
	TObjectPtr<class UPlayerInputDataAsset> PlayerInputDataAssetInternal = nullptr; //[B]

	/** Sounds data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Sounds Data Asset"))
	TObjectPtr<class USoundsDataAsset> SoundsDataAssetInternal = nullptr; //[B]

	/** Actor type and its associated class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Actors Data Assets"))
	TArray<TObjectPtr<class ULevelActorDataAsset>> ActorsDataAssetsInternal; //[B]
};
