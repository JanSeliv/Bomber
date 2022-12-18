﻿// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "PlayerInputDataAsset.generated.h"

class UMyInputMappingContext;

/**
* Contains all data that describe player input.
*/
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UPlayerInputDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the player input data asset. */
	static const UPlayerInputDataAsset& Get();

	/** Returns all input contexts contained in this data asset. */
	void GetAllInputContexts(TArray<const UMyInputMappingContext*>& OutInputContexts) const;

	/** Returns all gameplay input contexts contained in this data asset. */
	void GetAllGameplayInputContexts(TArray<const UMyInputMappingContext*>& OutGameplayInputContexts) const;

	/** Returns the overall amount of all gameplay input contexts. */
	UFUNCTION(BlueprintPure, Category = "C++")
	int32 GetGameplayInputContextsNum() const { return GameplayInputContextsInternal.Num(); }

	/** Returns the Enhanced Input Mapping Context of gameplay actions for specified local player.
	* @param LocalPlayerIndex The index of a local player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const UMyInputMappingContext* GetGameplayInputContext(int32 LocalPlayerIndex) const;

	/** Returns the Enhanced Input Mapping Context of actions on the Main Menu widget.
	* @see UPlayerInputDataAsset::MainMenuInputContextInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE UMyInputMappingContext* GetMainMenuInputContext() const { return MainMenuInputContextInternal; }

	/** Returns the Enhanced Input Mapping Context of actions on the In-Game Menu widget.
	  * @see UPlayerInputDataAsset:: */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE UMyInputMappingContext* GetInGameMenuInputContext() const { return InGameMenuInputContextInternal; }

	/** Returns the Enhanced Input Mapping Context of actions on the Settings widget.
	  * @see ::SettingsInputContextInternalInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE UMyInputMappingContext* GetSettingsInputContext() const { return SettingsInputContextInternalInternal; }

	/** Returns true if specified key is mapped to any gameplay input context. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Key"))
	bool IsMappedKey(const FKey& Key) const;

protected:
	/** Enhanced Input Mapping Contexts of gameplay input actions where any selected input can be remapped by player.
	 *  Are selectable classes instead of objects directly to solve next UE issues:
	 *  - to avoid changing data asset by MapKey, UnmapKey or RemapKey.
	 *  - to have outer for gameplay contexts to let GC to garbage it after context is serialized from config that contains overriden changes by remapping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Gameplay Input Context Classes", ShowOnlyInnerProperties))
	TArray<TSubclassOf<UMyInputMappingContext>> GameplayInputContextClassesInternal;

	/** Enhanced Input Mapping Context of actions on the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Main Menu Input Context", ShowOnlyInnerProperties))
	TObjectPtr<UMyInputMappingContext> MainMenuInputContextInternal = nullptr;

	/** Enhanced Input Mapping Context of actions on the Main Menu widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In-Game Menu Input Context", ShowOnlyInnerProperties))
	TObjectPtr<UMyInputMappingContext> InGameMenuInputContextInternal = nullptr;

	/** Enhanced Input Mapping Context of actions on the Settings widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Input Context", ShowOnlyInnerProperties))
	TObjectPtr<UMyInputMappingContext> SettingsInputContextInternalInternal = nullptr;

	/** Creates new contexts if is needed, is implemented to solve UE issues with remappings, see details below.
	 * @see UPlayerInputDataAsset::GameplayInputContextClassesInternal */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryCreateGameplayInputContexts() const;

private:
	/** Are created dynamically by specified input classes to solve UE issues with remappings, see details below.
	 * @see UPlayerInputDataAsset::GameplayInputContextClassesInternal */
	UPROPERTY(Transient)
	mutable TArray<TObjectPtr<class UMyInputMappingContext>> GameplayInputContextsInternal; //[G]
};