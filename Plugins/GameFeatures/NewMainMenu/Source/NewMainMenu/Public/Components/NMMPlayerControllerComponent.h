// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "NMMPlayerControllerComponent.generated.h"

class AMyPlayerController;

enum class ENMMState : uint8;

/**
 * Represents the Player Controller in the NewMain Menu module, where the Owner is Player Controller actor.
 * Is responsible for managing Main Menu inputs.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWMAINMENU_API UNMMPlayerControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UNMMPlayerControllerComponent();

	/** Returns Player Controller of this component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	AMyPlayerController* GetPlayerController() const;
	AMyPlayerController& GetPlayerControllerChecked() const;

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Returns loaded and cached Save Game Data of the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UNMMSaveGameData* GetSaveGameData() const { return SaveGameDataInternal; }

	/** Assigns existing Save Game Data to this component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSaveGameData(class USaveGame* NewSaveGameData);

	/** Enables or disables the input context during Cinematic Main Menu State. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCinematicInputContextEnabled(bool bEnable);

	/** Enables or disables Cinematic mouse settings from Player Input data asset. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCinematicMouseVisibilityEnabled(bool bEnabled);

	/** Enables or disables the input context according to new menu state. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetManagedInputContextsEnabled(ENMMState NewState);

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Contains loaded and cached Save Game Data of the Main Menu. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Save Game Data"))
	TObjectPtr<class UNMMSaveGameData> SaveGameDataInternal = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when the owning Actor begins play or when the component is created if the Actor has already begun play. */
	virtual void BeginPlay() override;

	/** Clears all transient data created by this component */
	virtual void OnUnregister() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called wen the Main Menu state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnNewMainMenuStateChanged(ENMMState NewState);

	/** Is listen to set Menu game state once first spot is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMainMenuSpotReady(class UNMMSpotComponent* MainMenuSpotComponent);

	/** Is called from AsyncLoadGameFromSlot once Save Game is loaded, or null if it failed to load. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnAsyncLoadGameFromSlotCompleted(const FString& SlotName, int32 UserIndex, class USaveGame* SaveGame);
};
