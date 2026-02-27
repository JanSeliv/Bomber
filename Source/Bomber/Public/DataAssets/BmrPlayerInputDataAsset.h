// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// Bomber
#include "Structures/BmrMouseVisibilitySettings.h"

#include "BmrPlayerInputDataAsset.generated.h"

class UBmrInputMappingContext;

struct FKey;

/**
 * Contains all data that describe player input.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrPlayerInputDataAsset final : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the player input data asset. */
	static const UBmrPlayerInputDataAsset& Get();

	/** Returns all input contexts contained in this data asset. */
	void GetAllInputContexts(TArray<const UBmrInputMappingContext*>& OutInputContexts) const;

	/** Returns all gameplay input contexts contained in this data asset. */
	void GetAllGameplayInputContexts(TArray<const UBmrInputMappingContext*>& OutGameplayInputContexts) const;

	/** Returns the overall amount of all gameplay input contexts. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetGameplayInputContextsNum() const { return GameplayInputContexts.Num(); }

	/** Returns the Enhanced Input Mapping Context of gameplay actions for specified local player.
	 * @param LocalPlayerIndex The index of a local player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const UBmrInputMappingContext* GetGameplayInputContext(int32 LocalPlayerIndex) const;

	/** Returns the Enhanced Input Mapping Context of actions on the In-Game Menu widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE UBmrInputMappingContext* GetInGameMenuInputContext() const { return InGameMenuInputContext; }

	/** Returns the Enhanced Input Mapping Context of actions on the Settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE UBmrInputMappingContext* GetSettingsInputContext() const { return SettingsInputContext; }

	/** Returns the mouse visibility settings by specified game state tag.
	 * @see UPlayerInputDataAsset::MouseVisibilitySettings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FBmrMouseVisibilitySettings& GetMouseVisibilitySettings(struct FBmrGameStateTag GameStateTag) const;

	/** Returns the mouse visibility settings by custom game state.
	 * @see UPlayerInputDataAsset::MouseVisibilitySettings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FBmrMouseVisibilitySettings& GetMouseVisibilitySettingsCustom(FName CustomGameState) const;

	/** Returns true if specified key is mapped to any gameplay input context. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext", AutoCreateRefTerm = "Key"))
	bool IsMappedKey(const UObject* WorldContext, const FKey& Key) const;

	/** Performs cleanup of transient data. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void EmptyGameplayInputContexts() const { GameplayInputContexts.Empty(); }

protected:
	/** Enhanced Input Mapping Contexts of gameplay input actions where any selected input can be remapped by player.
	 *  Are selectable classes instead of objects directly to solve next UE issues:
	 *  - to avoid changing data asset by MapKey, UnmapKey or RemapKey.
	 *  - to have outer for gameplay contexts to let GC to garbage it after context is serialized from config that contains overriden changes by remapping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TArray<TSubclassOf<UBmrInputMappingContext>> GameplayInputContextClasses;

	/** Enhanced Input Mapping Context of actions on the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<const UBmrInputMappingContext> InGameMenuInputContext = nullptr;

	/** Enhanced Input Mapping Context of actions on the Settings widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	TObjectPtr<const UBmrInputMappingContext> SettingsInputContext = nullptr;

	/** Determines mouse visibility behaviour per game states. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	TArray<FBmrMouseVisibilitySettings> MouseVisibilitySettings;

	/** Creates new contexts if is needed, is implemented to solve UE issues with remappings, see details below.
	 * @see UPlayerInputDataAsset::GameplayInputContextClasses */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void TryCreateGameplayInputContexts() const;

private:
	/** Are created dynamically by specified input classes to solve UE issues with remappings, see details below.
	 * @see UPlayerInputDataAsset::GameplayInputContextClasses */
	UPROPERTY(Transient)
	mutable TArray<TObjectPtr<class UBmrInputMappingContext>> GameplayInputContexts;
};