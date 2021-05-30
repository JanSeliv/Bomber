// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Structures/Cell.h"
#include "Structures/SettingsRow.h"
//---
#include "SingletonLibrary.generated.h"

/**
 * 	The static functions library
 */
UCLASS(Blueprintable, BlueprintType)
class USingletonLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FUpdateAI);
	/** Binds to update movements of each AI controller. */
	static FUpdateAI GOnAIUpdatedDelegate;

	/** Sets default values for this actor's properties */
	USingletonLibrary() = default;

	/** Returns a world of stored level map. */
	virtual class UWorld* GetWorld() const override;

	/* ---------------------------------------------------
	 *		Editor development
	 * --------------------------------------------------- */

#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE(FOnAnyDataAssetChanged);
	/** Will notify on any data asset changes. */
	static FOnAnyDataAssetChanged GOnAnyDataAssetChanged;
#endif //WITH_EDITOR

	/** Checks, is the current world placed in the editor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditor();

	/** Checks is the current world placed in the editor and the game not started yet. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditorNotPieWorld();

	/** Blueprint debug function, that prints messages to the log */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, AutoCreateRefTerm = "FunctionName,Message", WorldContext = "UObj", CallableWithoutWorldContext))
	static void PrintToLog(const UObject* UObj, const FString& FunctionName, const FString& Message = "");

	/** Remove all text renders of the Owner */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner"))
	static void ClearOwnerTextRenders(class AActor* Owner);

	/** Debug visualization by text renders. Has blueprint implementation. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure = false, Category = "C++", meta = (DevelopmentOnly, AdvancedDisplay = 2, AutoCreateRefTerm = "TextColor,RenderText,CoordinatePosition", DefaultToSelf = "Owner"))
	void AddDebugTextRenders(
		class AActor* Owner,
		const TSet<FCell>& Cells,
		const struct FLinearColor& TextColor, // used in the child native event
		bool& bOutHasCoordinateRenders,       // used in the child native event
		TArray<class UTextRenderComponent*>& OutTextRenderComponents,
		float TextHeight = 261.f,
		float TextSize = 124.f,
		const FString& RenderString = "",
		const FVector& CoordinatePosition = FVector::ZeroVector) const;

	/* ---------------------------------------------------
	 *		Static library functions
	 * --------------------------------------------------- */

	/** Returns the singleton, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static USingletonLibrary* GetSingleton();
	static FORCEINLINE const USingletonLibrary& Get() { return *GetSingleton(); };

	/** The Level Map getter, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AGeneratedMap* GetLevelMap();

	/** The Level Map setter. If the specified Level Map is not valid or is transient, find and set another one
	 *
	 * @param LevelMap The level map to set in the Library
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetLevelMap(class AGeneratedMap* LevelMap);

	/** Returns number of alive players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static int32 GetAlivePlayersNum();

	/** Returns the type of the current level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static ELevelType GetLevelType();

	/** Return rhe Bomber Game Instance, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UMyGameInstance* GetMyGameInstance();

	/** Returns the Bomber Game Mode, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyGameModeBase* GetMyGameMode();

	/** Returns the Bomber Game state, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyGameStateBase* GetMyGameState();

	/** Returns the Bomber Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyPlayerController* GetMyPlayerController();

	/** Returns the Bomber Player State for specified player, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyPlayerState* GetMyPlayerState(const class AController* Controller);

	/** Returns the player state of current controller. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyPlayerState* GetCurrentPlayerState();

	/** Returns the Bomber settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UMyGameUserSettings* GetMyGameUserSettings();

	/** Returns the settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class USettingsWidget* GetSettingsWidget();

	/** Returns the Camera Component used on level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UMyCameraComponent* GetLevelCamera();

	/** Returns the HUD actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyHUD* GetMyHUD();

	/** Returns controlled player character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class APlayerCharacter* GetControllablePlayer();

	/** Returns true if specified actor is a controllable player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsControllablePlayer(const class AActor* Player);

	/* ---------------------------------------------------
	 *		Structs functions
	 * --------------------------------------------------- */

	/** Returns the length of the one cell (a floor bound) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE float GetCellSize() { return FCell::CellSize; }

	/** Returns the zero cell (0,0,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE FCell GetZeroCell() { return FCell::ZeroCell; }

	/** Returns the zero cell (0,0,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static FORCEINLINE bool IsValidCell(const FCell& Cell) { return Cell; }

	/** Rotation of the input vector around the center of the Level Map to the same yaw degree
	 *
	 * @param Cell The cell, that will be rotated
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "VectorToRotate,AxisZ"))
	static FORCEINLINE struct FCell RotateCellAngleAxis(const FCell& Cell, const float& AxisZ)
	{
		return Cell.RotateAngleAxis(AxisZ);
	}

	/** Calculate the length between two cells
	 *
	 * @param C1 The first cell
	 * @param C2 The other cell
	 * @return The distance between to cells
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "C1,C2"))
	static FORCEINLINE float CalculateCellsLength(const struct FCell& C1, const struct FCell& C2)
	{
		return fabsf((C1.Location - C2.Location).Size()) / GetCellSize();
	}

	/** Find the average of an set of cells */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FCell GetCellArrayAverage(const TSet<struct FCell>& Cells);

	/** Returns empty settings row. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE FSettingsPicker GetEmptySettingsRow() { return FSettingsPicker::Empty; }

	/** Returns true if row is valid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "SettingsRow"))
	static FORCEINLINE bool IsValidSettingsRow(const FSettingsPicker& SettingsRow) { return SettingsRow.IsValid(); }

	/* ---------------------------------------------------
	*		EActorType bitmask functions
	* --------------------------------------------------- */

	/** Bitwise and(&) operation with bitmasks of actors types.
	 * Checks the actors types among each other between themselves */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "&"))
	static FORCEINLINE bool BitwiseActorTypes(
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 LBitmask,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 RBitmask)
	{
		return (LBitmask & RBitmask) != 0;
	}

	/* ---------------------------------------------------
	*		Data assets
	* --------------------------------------------------- */

	/** Returns the Levels Data Asset*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE class UGeneratedMapDataAsset* GetLevelsDataAsset() { return Get().LevelsDataAssetInternal; }

	/** Returns the UI Data Asset*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE class UUIDataAsset* GetUIDataAsset() { return Get().UIDataAssetInternal; }

	/** Returns the settings data.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE class USettingsDataAsset* GetSettingsDataAsset() { return Get().SettingsDataAssetInternal; }

	/** Returns the AI data.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE class UAIDataAsset* GetAIDataAsset() { return Get().AIDataAssetInternal; }

	/** Iterate ActorsDataAssets array and returns the found Level Actor class by specified data asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorClass"))
	static class ULevelActorDataAsset* GetDataAssetByActorClass(const TSubclassOf<AActor>& ActorClass);

	/** Iterate ActorsDataAssets array and returns the found Data Assets of level actors by specified types. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static void GetDataAssetsByActorTypes(
		TArray<class ULevelActorDataAsset*>& OutDataAssets,
		UPARAM(meta = (Bitmask, BitmaskEnum = "EActorType")) int32 ActorsTypesBitmask);

	/** Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class ULevelActorDataAsset* GetDataAssetByActorType(EActorType ActorType);

	/** Iterate ActorsDataAssets array and returns the found actor class by specified actor type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static TSubclassOf<AActor> GetActorClassByType(EActorType ActorType);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Is actor on persistent level.
	 * Is uproperty to be accessible in blueprints.
	 * Is transient to not serialize it since will be set by AGeneratedMap::OnConstruction().
	 * Is stored in singleton, so has weak reference field to be garbage collected on loading another maps where that actor does not exist. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Level Map"))
	TWeakObjectPtr<class AGeneratedMap> LevelMapInternal; //[G]

	/** Contains properties to setup the generated level. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Levels Data Asset"))
	class UGeneratedMapDataAsset* LevelsDataAssetInternal; //[B]

	/** Settings data. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Data Asset"))
	class USettingsDataAsset* SettingsDataAssetInternal; //[B]

	/** Contains properties to setup UI. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "UI Data Asset"))
	class UUIDataAsset* UIDataAssetInternal; //[B]

	/** AI data. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "AI Data Asset"))
	class UAIDataAsset* AIDataAssetInternal; //[B]

	/** Actor type and its associated class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Actors Data Assets"))
	TArray<class ULevelActorDataAsset*> ActorsDataAssetsInternal; //[B]
};
