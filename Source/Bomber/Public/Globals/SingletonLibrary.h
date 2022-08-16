// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
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

	/** Remove all text renders of the Owner */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner"))
	static void ClearOwnerTextRenders(class AActor* Owner);

	/** Debug visualization by text renders. Has blueprint implementation. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure = false, Category = "C++", meta = (DevelopmentOnly, AdvancedDisplay = 2, AutoCreateRefTerm = "TextColor,RenderString,CoordinatePosition", DefaultToSelf = "Owner"))
	void AddDebugTextRenders(class AActor* Owner, const TSet<FCell>& Cells, const FLinearColor& TextColor, bool& bOutHasCoordinateRenders, TArray<class UTextRenderComponent*>& OutTextRenderComponents, float TextHeight, float TextSize, const FString& RenderString, const FVector& CoordinatePosition) const;
	static void AddDebugTextRenders(
		class AActor* Owner,
		const TSet<FCell>& Cells,
		const FLinearColor& TextColor,
		float TextHeight = 261.f,
		float TextSize = 124.f,
		const FString& RenderString = TEXT(""),
		const FVector& CoordinatePosition = FVector::ZeroVector);

	/* ---------------------------------------------------
	 *		Static library functions
	 * --------------------------------------------------- */

	/** Returns the singleton, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static USingletonLibrary* GetSingleton();
	static FORCEINLINE const USingletonLibrary& Get() { return *GetSingleton(); }

	/** Returns true if specified actor is the Bomber Level Actor (player, box, wall or item). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsLevelActor(const class AActor* Actor);

	/** Iterates the current world to find an actor by specified class.
	 * @warning use this functions carefully, try always avoid it and get cached actor instead. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static AActor* GetActorOfClass(TSubclassOf<AActor> ActorClass);

	/** Iterates the current world to find an actor by specified class. */
	template <typename T>
	static FORCEINLINE T* GetActorOfClass(TSubclassOf<AActor> ActorClass) { return Cast<T>(GetActorOfClass(ActorClass)); }

	/** Returns true if game was started. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool HasWorldBegunPlay();

	/** Returns true if this instance is server. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsServer();

	/** The Level Map setter. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetLevelMap(class AGeneratedMap* LevelMap);

	/** Returns number of alive players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static int32 GetAlivePlayersNum();

	/** Returns the type of the current level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static ELevelType GetLevelType();

	/* ---------------------------------------------------
	 *		Framework pointer getters
	 * --------------------------------------------------- */

	/** The Level Map getter, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AGeneratedMap* GetLevelMap();

	/** Return rhe Bomber Game Instance, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UMyGameInstance* GetMyGameInstance();

	/** Returns the Bomber Game Mode, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyGameModeBase* GetMyGameMode();

	/** Returns the Bomber Game state, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyGameStateBase* GetMyGameState();

	/** Returns the specified Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyPlayerController* GetMyPlayerController(int32 PlayerIndex);

	/** Returns the local Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyPlayerController* GetLocalPlayerController();

	/** Returns the Bomber Player State for specified player, nullptr otherwise. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyPlayerState* GetMyPlayerState(const class APawn* Pawn);

	/** Returns the player state of current controller. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyPlayerState* GetLocalPlayerState();

	/** Returns the Bomber settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UMyGameUserSettings* GetMyGameUserSettings();

	/** Returns the Settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class USettingsWidget* GetSettingsWidget();

	/** Returns the Camera Component used on level. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UMyCameraComponent* GetLevelCamera();

	/** Returns the HUD actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AMyHUD* GetMyHUD();

	/** Returns the Main Menu widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UMainMenuWidget* GetMainMenuWidget();

	/** Returns the In-Game widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UInGameWidget* GetInGameWidget();

	/** Returns the In-Game Menu widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UInGameMenuWidget* GetInGameMenuWidget();

	/** Returns specified player character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class APlayerCharacter* GetPlayerCharacter(int32 PlayerIndex);

	/** Returns controlled player character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class APlayerCharacter* GetLocalPlayerCharacter();

	/** Returns the Sounds Manager. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class USoundsManager* GetSoundsManager();

	/** Returns the Pool Manager of the game that is used to reuse created objects. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class UPoolManager* GetPoolManager();

	/** Returns the Levels Data Asset*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static const FORCEINLINE class UDataAssetsContainer* GetDataAssetsContainer() { return Get().DataAssetsContainerInternal; }

	/* ---------------------------------------------------
	 *		Structs functions
	 * --------------------------------------------------- */

	/** Returns the length of the one cell (a floor bound) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE float GetCellSize() { return FCell::CellSize; }

	/** Returns the zero cell (0,0,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static const FORCEINLINE FCell& GetZeroCell() { return FCell::ZeroCell; }

	/** Returns the zero cell (0,0,0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static FORCEINLINE bool IsValidCell(const FCell& Cell) { return Cell.IsValid(); }

	/** Rotation of the input vector around the center of the Level Map to the same yaw degree
	 *
	 * @param Cell The cell, that will be rotated
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Cell"))
	static FORCEINLINE FCell RotateCellAngleAxis(const FCell& Cell, float AxisZ)
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
	static float CalculateCellsLength(const FCell& C1, const FCell& C2);

	/** Find the average of an set of cells */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FCell GetCellArrayAverage(const TSet<FCell>& Cells);

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


protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Is actor on persistent level.
	 * Is uproperty to be accessible in blueprints.
	 * Is transient to not serialize it since will be set by AGeneratedMap::OnConstruction().
	 * Is stored in singleton, so has weak reference field to be garbage collected on loading another maps where that actor does not exist. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Level Map"))
	TWeakObjectPtr<class AGeneratedMap> LevelMapInternal = nullptr; //[G]

	/** Contains the Pool Manager of the game that is used to reuse created objects. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Pool Manager"))
	TWeakObjectPtr<class UPoolManager> PoolManagerInternal = nullptr; //[G]

	/** Contains core data of the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Data Assets Container"))
	TObjectPtr<class UDataAssetsContainer> DataAssetsContainerInternal = nullptr; //[B]
};
