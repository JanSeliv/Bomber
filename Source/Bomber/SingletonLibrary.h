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
#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE(FPushNongeneratedToMap);
	/** @defgroup [PIE] Runs only in editor
	 * Owners Map Components binds to updating on the Level Map to this delegate
	 * The Level Map broadcasts this delegate after own generation
	 * @see class UMapComponent
	 */
	FPushNongeneratedToMap OnActorsUpdatedDelegate;
#endif  //WITH_EDITOR [PIE]

	/** @addtogroup AI
	 * @addtogroup [Editor]Editor
	 * Call all signed as bShouldShowRenders AI characters
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	static void BroadcastAiUpdating();

	/** Blueprint debug function, that prints messages to the log */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, AutoCreateRefTerm = "FunctionName,Message"))  //
	static FORCEINLINE int32 PrintToLog(const UObject* UObj, const FString& FunctionName, const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t %s \t %s \t %s"), (UObj ? *UObj->GetName() : TEXT("")), *FunctionName, *Message);
		return 0;
	}

	/** @addtogroup [Editor]Editor
	 *Remove all text renders of the Owner */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, HidePin = "Owner", DefaultToSelf = "Owner"))
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
	static void AddDebugTextRenders(
		class AActor* Owner,
		const TSet<struct FCell>& Cells,
		const struct FLinearColor& TextColor = FLinearColor::Black);
#endif  //WITH_EDITOR [Editor]

	/** 
	 * The singleton getter
	 * @return The singleton, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static USingletonLibrary* GetSingleton();

	/** The Level Map getter */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static FORCEINLINE class AGeneratedMap* const GetLevelMap()
	{
		return ensure(GEngine && GetSingleton()) ? GetSingleton()->LevelMap_ : nullptr;
	}

	/** @defgroup Cell_BP_Functions The group with cell functions that used in blueprints
	 * The custom make node of the FCell struct
	 * Used as a blueprint implementation of the default MakeStruct node
	 * 
	 * @param Actor Finding the closest cell by actor
	 * @return The found cell
	 * @warning Deprecated, temporary function
	 * @warning Has blueprint implementation
	 * @todo Rewrite to C++ FCell()
	 * @todo #5 Nearest cell: replace thisCell to the FoundCell and with_editor things
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "C++", meta = (CompactNodeTitle = "MakeCell"))
	struct FCell MakeCell(const class AActor* Actor) const;

	/** @addtogroup Cell_BP_Functions
	 * @return The length of one cell (a floor bound)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE float GetGridSize()
	{
		return 200.f;
	}

	/** @addtogroup Cell_BP_Functions
	 * Calculate the length between two cells
	 *
	 * @param C1 The first cell
	 * @param C2 The other one cell
	 * @return The distance between to cells
	 * @todo FVector::Dist(X, Y);
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE float CalculateCellsLength(const struct FCell& C1, const struct FCell& C2)
	{
		return fabsf((C1.Location - C2.Location).Size()) / GetGridSize();
	}

	/** @addtogroup Cell_BP_Functions
	 * Rotation of the input vector around the center of the Level Map to the same yaw degree
	 * 
	 * @param VectorToRotate The vector, that will be rotated
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "VectorToRotate,AxisZ"))
	static FCell CalculateVectorAsRotatedCell(
		const FVector& VectorToRotate,
		const float& AxisZ);

	/** @addtogroup actor_types
	 * Bitwise and(&) operation with the bitmask of actor types*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorType,Bitmask", CompactNodeTitle = "&"))
	static FORCEINLINE bool ContainsActorType(
		const EActorTypeEnum& ActorType,
		UPARAM(meta = (Bitmask, BitmaskEnum = EActorTypeEnum)) const int32& Bitmask)
	{
		return (TO_FLAG(ActorType) & Bitmask) != 0;
	}

	/** @addtogroup actor_types
	 * Check for the content of the actor type among incoming types
	 * 
	 * @param Actor The level actor for comparison
	 * @param Bitmask Enumerations of actors types
	 * @return true if bitmask contains the actor's type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "Bitmask"))
	static bool IsActorInTypes(
		const AActor* Actor,
		UPARAM(meta = (Bitmask, BitmaskEnum = EActorTypeEnum)) const int32& Bitmask);

	/** @addtogroup actor_types
	 * Find the class value by actor type key in the ActorTypesByClasses dictionary */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorType"))
	static TSubclassOf<AActor> FindClassByActorType(const EActorTypeEnum& ActorType);

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
