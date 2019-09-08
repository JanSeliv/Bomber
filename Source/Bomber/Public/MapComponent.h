// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Cell.h"
#include "Components/ActorComponent.h"

#include "MapComponent.generated.h"

/** Typedef to allow for some nicer looking sets of map components */
typedef TSet<class UMapComponent*> FMapComponents;

/** 
 * These components manage their level actors updates on the level map in case of any changes that allow to:
 * -  Free location and rotation of the level map in the editor time:
 * - Prepare in advance the level actors in the editor time:
 * Same calls and initializations for each of the level map actors
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++")
	EActorType ActorType = EActorType::None;  //[i]

#if WITH_EDITORONLY_DATA  // bShouldShowRenders
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (DevelopmentOnly))
	bool bShouldShowRenders = false;
#endif  //WITH_EDITORONLY_DATA bShouldShowRenders

	/* ---------------------------------------------------
	 *	Map Component's public functions
	 * --------------------------------------------------- */

	/** Sets default values for this component's properties */
	UMapComponent();

	/** Updates a owner's state. Should be called in the owner's OnConstruction event. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnMapComponentConstruction();

	/** Getter of the Cell.
	 * @return the cell location of an owner. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE struct FCell GetCell() const { return Cell; }

	/** Finds and sets to the owner's map component a non-zero cell.
	 * @return true if the cell is free from other level actors. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	FORCEINLINE bool UpdateCell()
	{
		Cell = FCell(this);
		return Cell.bWasFound;
	}

protected:
	/** Owner's cell location on the Level Map */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	struct FCell Cell;  //[G]

	/* ---------------------------------------------------
	 *	Map Component's protected functions
	 * --------------------------------------------------- */

	/** Called when a component is registered (not loaded) */
	virtual void OnRegister() override;

	/** Called when a component is destroyed for removing the owner from the Level Map. */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
};
