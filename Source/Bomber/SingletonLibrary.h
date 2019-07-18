// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SingletonLibrary.generated.h"

UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USingletonLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPushNongeneratedToMap);
	/** @defgroup [Editor]Editor Runs only in editor
	 * Owners Map Components binds to updating on the Level Map to this delegate
	 * The Level Map broadcasts this delegate after own generation
	 * @see class UMapComponent
	 */
	UPROPERTY(BlueprintCallable, Category = "C++")
	FPushNongeneratedToMap OnActorsUpdatedDelegate;

	/** @addtogroup AI
	 * @addtogroup [Editor]Editor
	 * Call all signed as bShouldShowRenders AI characters
	 * @param Owner The called owner
	 * @warning Is not static for OnDestroyed binding
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	void BroadcastAiUpdating(AActor* Owner);

	/** @addtogroup [Editor]Editor
	 *Remove all text renders of the Owner */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++", meta = (DevelopmentOnly, HidePin = "Owner", DefaultToSelf = "Owner"))
	static void ClearOwnerTextRenders(class AActor* Owner);

	/** @addtogroup [Editor]Editor
	 * Debug visualization by text renders
	 * @warning PIE only
	 * @warning Has blueprint implementation
     */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure = false, Category = "C++", meta = (DevelopmentOnly, HidePin = "Owner", DefaultToSelf = "Owner", AdvancedDisplay = 2, AutoCreateRefTerm = "TextColor,RenderText,CoordinatePosition"))
	void AddDebugTextRenders(
		class AActor* Owner,
		const TSet<struct FCell>& Cells,
		const struct FLinearColor& TextColor,
		bool& bOutHasCoordinateRenders,
		TArray<class UTextRenderComponent*>& OutTextRenderComponents,
		float TextHeight = 261.0f,
		float TextSize = 124.0f,
		const FText& RenderText = FText::GetEmpty(),
		const FVector& CoordinatePosition = FVector(0.f)) const;
#if WITH_EDITOR
	/** @addtogroup [Editor]Editor
	 *Shortest overloading of debugging visualization*/
	static FORCEINLINE void AddDebugTextRenders(
		class AActor* Owner,
		const TSet<struct FCell>& Cells,
		const struct FLinearColor& TextColor = FLinearColor::Black)
	{
		bool bOutBool = false;
		TArray<class UTextRenderComponent*> OutArray{};
		GetSingleton()->AddDebugTextRenders(Owner, Cells, TextColor, bOutBool, OutArray);
	}
#endif  //WITH_EDITOR [Editor]

	/** @addtogroup cell_functions
	 * The custom make node of the FCell struct that used as a blueprint implementation of the default MakeStruct node
	 * @param Actor Finding the closest cell by actor
	 * @return The found cell
	 * @warning Deprecated, temporary function
	 * @warning Has blueprint implementation
	 * @todo Rewrite to C++ FCell()
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "toCell"))
	FORCEINLINE struct FCell MakeCell(const class AActor* Actor) const;

	/** 
	 * The singleton getter
	 * @return The singleton, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE USingletonLibrary* GetSingleton()
	{
		USingletonLibrary* Singleton = nullptr;
		if (GEngine) Singleton = Cast<USingletonLibrary>(GEngine->GameSingleton);
		ensureMsgf(Singleton != nullptr, TEXT("The Singleton is null"));
		return Singleton;
	}

	/** @addtogroup cell_functions
	 * @return The length of one cell (a floor bound)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get Grid Size"))
	static FORCEINLINE float GetFloorLength()
	{
		return 200.0;
	}

	/** @addtogroup cell_functions
	 * Calculate the length between two cells
	 * @param X The first cell
	 * @param Y The other one cell
	 * @return The distance between to cells
	 * @todo FVector::Dist(X, Y);
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE float CalculateCellsLength(const struct FCell& X, const struct FCell& Y)
	{
		return (fabsf((X.Location - Y.Location).Size()) / GetFloorLength());
	}

	/** The Level Map getter */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static class AGeneratedMap* const GetLevelMap(UObject* WorldContextObject);

	/** @addtogroup actor_types
	 * Find the actor type key by class value in the ActorTypesByClasses dictionary */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorClass"))
	static FORCEINLINE EActorTypeEnum FindActorTypeByClass(const TSubclassOf<AActor>& ActorClass)
	{
		const EActorTypeEnum* FoundActorType = GetSingleton()->ActorTypesByClasses.FindKey(ActorClass);
		return (FoundActorType != nullptr ? *FoundActorType : EActorTypeEnum::None);
	}

	/** @addtogroup actor_types
	 * Find the class value by actor type key in the ActorTypesByClasses dictionary */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorType"))
	static FORCEINLINE TSubclassOf<AActor> FindClassByActorType(const EActorTypeEnum& ActorType)
	{
		const TSubclassOf<AActor>* ActorClass = GetSingleton()->ActorTypesByClasses.Find(ActorType);
		return (ActorClass != nullptr ? *ActorClass : nullptr);
	}

protected:
	/** The reference to the AGeneratedMap actor*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	class AGeneratedMap* LevelMap_;

	/** @addtogroup actor_types
	 * Type and its class as associated pairs  */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	TMap<EActorTypeEnum, TSubclassOf<AActor>> ActorTypesByClasses;

	/** Access to the Level Map to keep an in gaming valid reference */
	friend class AGeneratedMap;
};
