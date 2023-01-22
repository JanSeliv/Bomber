// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "Bomber.h"
#include "Structures/Cell.h"
//---
#include "SingletonLibrary.generated.h"

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

	/** Returns implemented Game Viewport Client on the project side. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static class UMyGameViewportClient* GetGameViewportClient();

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
};
