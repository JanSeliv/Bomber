// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "MyCharacter.h"

#include "MyAiCharacter.generated.h"

UCLASS()
class BOMBER_API AMyAiCharacter final : public AMyCharacter
{
	GENERATED_BODY()
public:
	/** @defgroup AI Functions of AI-characters */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void UpdateAI();

#if WITH_EDITORONLY_DATA
	/** @addtogroup AI
	 * Mark updating visualization(text renders) of the bot's movements in the editor
	 * @warning Editor only
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	bool bShouldShowRenders;
#endif  //WITH_EDITORONLY_DATA

protected:
	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/** @addtogroup AI
	 * Cell position of current path segment's end */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	struct FCell AiMoveTo;
};
