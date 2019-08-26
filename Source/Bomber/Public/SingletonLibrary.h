// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SingletonLibrary.generated.h"

/**
 * 	The static functions library  
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API USingletonLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *			Editor development functions
	 * --------------------------------------------------- */

#if WITH_EDITOR  // OnActorsUpdatedDelegate [IsEditorNotPieWorld]
	DECLARE_MULTICAST_DELEGATE(FPushNongeneratedToMap);
	/* Owners Map Components binds to updating on the Level Map to this delegate */
	FPushNongeneratedToMap OnActorsUpdatedDelegate;
#endif  //WITH_EDITOR OnActorsUpdatedDelegate [IsEditorNotPieWorld]

	/** Calls before generation preview actors to updating of all dragged to the Level Map actors. PIE only*/
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	static void BroadcastActorsUpdating();

	/** Calls all signed as bShouldShowRenders AI characters  */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly))
	static void BroadcastAiUpdating();

	/** Checks, that this actor placed in the editor world and the game is not started yet */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static bool IsEditorNotPieWorld();

	/** Blueprint debug function, that prints messages to the log */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, AutoCreateRefTerm = "FunctionName,Message"))
	static FORCEINLINE int32 PrintToLog(const UObject* UObj, const FString& FunctionName, const FString& Message = "")
	{
		UE_LOG(LogTemp, Warning, TEXT("\t %s \t %s \t %s"), (UObj ? *UObj->GetName() : TEXT("nullptr")), *FunctionName, *Message);
		return 0;
	}

	/** Remove all text renders of the Owner */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DevelopmentOnly, DefaultToSelf = "Owner"))
	static void ClearOwnerTextRenders(class AActor* Owner);

	/** 
	 * Debug visualization by text renders
	 * @warning PIE only
	 * @warning Has blueprint implementation
     */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure = false, Category = "C++", meta = (DevelopmentOnly, AdvancedDisplay = 2, AutoCreateRefTerm = "TextColor,RenderText,CoordinatePosition"))
	void AddDebugTextRenders(
		class AActor* Owner,
		const TSet<struct FCell>& Cells,
		const struct FLinearColor& TextColor,
		bool& bOutHasCoordinateRenders,
		TArray<class UTextRenderComponent*>& OutTextRenderComponents,
		float TextHeight,
		float TextSize,
		const FText& RenderText,
		const FVector& CoordinatePosition) const;

#if WITH_EDITOR  // AddDebugTextRenders
	/** Shortest static overloading of debugging visualization without outer params */
	static void AddDebugTextRenders(
		class AActor* Owner,
		const TSet<struct FCell>& Cells,
		const struct FLinearColor& TextColor = FLinearColor::Black,
		float TextHeight = 261.0f,
		float TextSize = 124.0f,
		const FString& RenderString = "",
		const FVector& CoordinatePosition = FVector::ZeroVector);
#endif  //WITH_EDITOR AddDebugTextRenders

	/* ---------------------------------------------------
	 *			Static library functions
	 * --------------------------------------------------- */

	/** Returns the singleton, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static USingletonLibrary* GetSingleton();

	/** The Level Map getter, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static class AGeneratedMap* GetLevelMap();

	/**
	 * The Level Map setter
	 * if input Level Map is no valid or is transient, find and set another one
	 * 
	 * @param LevelMap The level map to set in the Library
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetLevelMap(class AGeneratedMap* LevelMap);

	/** Contains a data of standalone and PIE games, nullptr otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (WorldContext = "WorldContextObject"))
	static class UMyGameInstance* GetMyGameInstance(const UObject* WorldContextObject);

	/* ---------------------------------------------------
	 *			FCell blueprint functions
	 * --------------------------------------------------- */

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
	 * @param C2 The other cell
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
	static struct FCell CalculateVectorAsRotatedCell(
		const FVector& VectorToRotate,
		const float& AxisZ);

	/* ---------------------------------------------------
	*			EActorTypeEnum bitmask functions
	* --------------------------------------------------- */

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
	 * Find the class value by actor type key in the ActorTypesByClasses_ dictionary */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorType"))
	static TSubclassOf<AActor> FindClassByActorType(const EActorTypeEnum& ActorType);

protected:
	/* ---------------------------------------------------
	*			Protected properties of the Singleton
	* --------------------------------------------------- */

	/** The reference to the AGeneratedMap actor*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	TSoftObjectPtr<class AGeneratedMap> LevelMap_;

	/** Type and its class as associated pairs  */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	TMap<EActorTypeEnum, TSubclassOf<AActor>> ActorTypesByClasses_;
};
