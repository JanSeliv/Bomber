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
 * Utility structure to display cells
 * @see USingletonLibrary::DisplayCells()
 */
USTRUCT(BlueprintType)
struct BOMBER_API FDisplayCellsParams
{
	GENERATED_BODY()

	/** Default params to display cells. */
	static const FDisplayCellsParams EmptyParams;

	/** Color of displayed text.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FLinearColor TextColor = FLinearColor::White;

	/** Height offset for displayed text above the cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float TextHeight = 261.f;

	/** Size of displayed text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	float TextSize = 124.f;

	/** Addition text to mark the cell, keep it short like 1-2 symbols. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FName RenderString = NAME_None;

	/** Offset for the Render String. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FVector CoordinatePosition = FVector::ZeroVector;

	/** Set true to remove all displays that were added on that owner before. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	bool bClearPreviousDisplays = false;
};

/**
 * 	The static functions library
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USingletonLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	USingletonLibrary() = default;

	/** Returns a world of stored level map. */
	virtual class UWorld* GetWorld() const override;

	/* ---------------------------------------------------
	 *		Static library functions
	 * --------------------------------------------------- */

	/** Returns the singleton, nullptr otherwise */
	UFUNCTION(BlueprintPure, Category = "C++")
	static USingletonLibrary* GetSingleton();
	static const FORCEINLINE USingletonLibrary& Get() { return *GetSingleton(); }

	/** Iterates the current world to find an actor by specified class.
	 * @warning use this functions carefully, try always avoid it and get cached actor instead. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static AActor* GetActorOfClass(TSubclassOf<AActor> ActorClass);

	/** Iterates the current world to find an actor by specified class. */
	template <typename T>
	static FORCEINLINE T* GetActorOfClass(TSubclassOf<AActor> ActorClass) { return Cast<T>(GetActorOfClass(ActorClass)); }

	/** Returns true if game was started. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool HasWorldBegunPlay();

	/** Returns true if this instance is server. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsServer();

	/** The Level Map setter. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetLevelMap(class AGeneratedMap* LevelMap);

	/** Returns number of alive players. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static int32 GetAlivePlayersNum();

	/** Returns the type of the current level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static ELevelType GetLevelType();

	/* ---------------------------------------------------
	 *		Framework pointer getters
	 * --------------------------------------------------- */

	/** The Level Map getter, nullptr otherwise */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (Keywords = "Generated"))
	static class AGeneratedMap* GetLevelMap();

	/** Return rhe Bomber Game Instance, nullptr otherwise */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UMyGameInstance* GetMyGameInstance();

	/** Returns the Bomber Game Mode, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyGameModeBase* GetMyGameMode();

	/** Returns the Bomber Game state, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyGameStateBase* GetMyGameState();

	/** Returns the specified Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyPlayerController* GetMyPlayerController(int32 PlayerIndex);

	/** Returns the local Player Controller, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyPlayerController* GetLocalPlayerController();

	/** Returns the Bomber Player State for specified player, nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyPlayerState* GetMyPlayerState(const class APawn* Pawn);

	/** Returns the player state of current controller. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyPlayerState* GetLocalPlayerState();

	/** Returns the Bomber settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UMyGameUserSettings* GetMyGameUserSettings();

	/** Returns the Settings widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class USettingsWidget* GetSettingsWidget();

	/** Returns the Camera Component used on level. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UMyCameraComponent* GetLevelCamera();

	/** Returns the HUD actor. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class AMyHUD* GetMyHUD();

	/** Returns the Main Menu widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UMainMenuWidget* GetMainMenuWidget();

	/** Returns the In-Game widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UInGameWidget* GetInGameWidget();

	/** Returns the In-Game Menu widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UInGameMenuWidget* GetInGameMenuWidget();

	/** Returns specified player character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class APlayerCharacter* GetPlayerCharacter(int32 PlayerIndex);

	/** Returns controlled player character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class APlayerCharacter* GetLocalPlayerCharacter();

	/** Returns the Sounds Manager. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class USoundsManager* GetSoundsManager();

	/** Returns the Pool Manager of the game that is used to reuse created objects. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UPoolManager* GetPoolManager();

	/** Returns the Levels Data Asset*/
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class UDataAssetsContainer* GetDataAssetsContainer();

	/* ---------------------------------------------------
	*		EActorType functions
	* --------------------------------------------------- */

	/** Bitwise and(&) operation with bitmasks of actors types.
	 * Checks the actors types among each other between themselves */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "&"))
	static FORCEINLINE bool BitwiseActorTypes(
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 LBitmask,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 RBitmask)
	{
		return (LBitmask & RBitmask) != 0;
	}

	/** Returns Actor Type of specified actor, None is not level actor. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static EActorType GetActorType(const AActor* Actor);

	/** Returns true if specified actor is the Bomber Level Actor (player, box, wall or item). */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsLevelActor(const AActor* Actor);

	/** Returns true if specified level actor has at least one specified type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static bool IsActorHasAnyMatchingType(
		const AActor* Actor,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Bomber.EActorType")) int32 ActorsTypesBitmask);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Is actor on persistent level.
	 * Is uproperty to be accessible in blueprints.
	 * Is transient to not serialize it since will be set by AGeneratedMap::OnConstruction().
	 * Is stored in singleton, so has weak reference field to be garbage collected on loading another maps where that actor does not exist. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Level Map"))
	TWeakObjectPtr<class AGeneratedMap> LevelMapInternal = nullptr;

	/** Contains the Pool Manager of the game that is used to reuse created objects. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Pool Manager"))
	TWeakObjectPtr<class UPoolManager> PoolManagerInternal = nullptr;

	/** Contains core data of the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Data Assets Container"))
	TObjectPtr<class UDataAssetsContainer> DataAssetsContainerInternal = nullptr;

	/* ---------------------------------------------------
	 *		Editor development
	 * --------------------------------------------------- */
public:
#if WITH_EDITORONLY_DATA
	DECLARE_MULTICAST_DELEGATE(FOnAnyDataAssetChanged);
	/** Will notify on any data asset changes. */
	static FOnAnyDataAssetChanged GOnAnyDataAssetChanged;

	DECLARE_MULTICAST_DELEGATE(FUpdateAI);
	/** Binds to update movements of each AI controller. */
	static FUpdateAI GOnAIUpdatedDelegate;
#endif //WITH_EDITOR

	/** Remove all text renders of the Owner */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner"))
	static void ClearDisplayedCells(const UObject* Owner);

	/** Display coordinates of specified cells on the level. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner", AdvancedDisplay = 2, AutoCreateRefTerm = "Params"))
	static void DisplayCells(UObject* Owner, const TSet<FCell>& Cells, const FDisplayCellsParams& Params);

	/** Display only specified cell. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner", AdvancedDisplay = 2, AutoCreateRefTerm = "Params"))
	static void DisplayCell(UObject* Owner, const FCell& Cell, const FDisplayCellsParams& Params) { DisplayCells(Owner, {Cell}, Params); }

protected:
	/** Debug visualization by text renders. Has blueprint implementation
	 * @TODO Move blueprint implementation to code, get rif of this function and move DisplayCell functions to UCellsUtilsLibrary. */
	UFUNCTION(BlueprintNativeEvent, meta = (DevelopmentOnly, BlueprintProtected, AdvancedDisplay = 2, AutoCreateRefTerm = "TextColor,RenderString,CoordinatePosition", DefaultToSelf = "Owner"))
	void AddDebugTextRenders(class AActor* Owner, const TSet<FCell>& Cells, const FLinearColor& TextColor, bool& bOutHasCoordinateRenders, TArray<class UTextRenderComponent*>& OutTextRenderComponents, float TextHeight, float TextSize, const FString& RenderString, const FVector& CoordinatePosition) const;
};
