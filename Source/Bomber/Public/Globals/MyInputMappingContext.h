// Copyright (c) Yevhenii Selivanov

#pragma once

#include "InputMappingContext.h"
//---
#include "Bomber.h"
#include "InputCoreTypes.h"
//---
#include "MyInputMappingContext.generated.h"

/**
 * Contains specific for this project data and is intended to with UMyInputAction
 */
UCLASS(Blueprintable, Const, AutoExpandCategories=("C++"))
class BOMBER_API UMyInputMappingContext final : public UInputMappingContext
{
	GENERATED_BODY()

public:
	/** Returns the priority of the context. If higher, then block the same consumed inputs other contexts with lower priorities.
	 * @see UMyInputMappingContext::ContextPriorityInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetContextPriority() const { return ContextPriorityInternal; }

	/** Returns the game states for which this input context is active.
	 * @see UMyInputMappingContext::ActiveForStatesInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetChosenGameStatesBitmask() const { return ActiveForStatesInternal; }

	/** Returns all input actions set in mappings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetInputActions(TArray<class UMyInputAction*>& OutInputActions) const;

	/** Returns mappings by specified input action.
	 * @param OutMappings Contains found mappings
	 * @param InputAction Action by that*/
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetMappingsByInputAction(TArray<FEnhancedActionKeyMapping>& OutMappings, const class UMyInputAction* InputAction) const;

	/** Returns all mappings where bIsPlayerMappable is true. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetAllMappings(TArray<FEnhancedActionKeyMapping>& OutMappableData) const;

	/** Unmap previous key and map new one.
	 * @param InInputAction The input action to be remapped.
	 * @param NewKey The key to set.
	 * @param PrevKey The key need to remap.
	 * @return false if something went wrong, f.e the specified key is already mapped to any gameplay context. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "NewKey,PrevKey"))
	bool RemapKey(const class UInputAction* InInputAction, const FKey& NewKey, const FKey& PrevKey);
	static bool RemapKey(const UMyInputMappingContext* InContext, const FEnhancedActionKeyMapping& InMapping, const FKey& NewKey);

protected:
	/** If higher, then block the same consumed inputs other contexts with lower priorities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Context Priority"))
	int32 ContextPriorityInternal = 0;

	/** Set the game states for which this input context should be active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Active For States", Bitmask, BitmaskEnum = "/Script/Bomber.ECurrentGameState"))
	int32 ActiveForStatesInternal = TO_FLAG(ECGS::None);

#if WITH_EDITOR
	/** Implemented to save input configs as well. */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif // WITH_EDITOR
};
