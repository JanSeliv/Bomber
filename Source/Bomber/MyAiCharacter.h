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
	///[AiDelegate]#if WITH_EDITOR
	///[AiDelegate]	/** @addtogroup AI
	///[AiDelegate] 	 * Called when the bShouldShowRenders on this character has been modified externally
	///[AiDelegate] 	 * Binding or unbinding render updates of render AI on creating\destroying elements
	///[AiDelegate] 	 * @param PropertyChangedEvent The property that was modified
	///[AiDelegate] 	 * @see USingletonLibrary::OnRenderAiUpdatedDelegate
	///[AiDelegate] 	 * @warning Editor only
	///[AiDelegate] 	 */
	///[AiDelegate]	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) final;
	///[AiDelegate]#endif

	/** @addtogroup AI
	 * Cell position of current path segment's end */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	struct FCell AiMoveTo;
};
