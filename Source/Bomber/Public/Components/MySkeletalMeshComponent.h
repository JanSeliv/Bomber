// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Components/SkeletalMeshComponent.h"
//---
#include "MySkeletalMeshComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMySkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UMySkeletalMeshComponent();

	/** Attach all FAttachedMesh UPlayerRow::PlayerProps to specified parent mesh. */
	UFUNCTION(BlueprintCallable, Category = "C++")
    void AttachProps(const class UPlayerRow* PlayerRow);
};
