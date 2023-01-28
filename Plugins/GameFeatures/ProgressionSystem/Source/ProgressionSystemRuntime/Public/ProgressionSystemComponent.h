// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "ProgressionSystemComponent.generated.h"

/**
 * Implements the core logic on project about Progression System.
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROGRESSIONSYSTEMRUNTIME_API UProgressionSystemComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UProgressionSystemComponent();

protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;
};
