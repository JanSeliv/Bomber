// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Bomber.h"
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

protected:
	/** Current level type of attached meshes. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "MeshesLevelType"))
	ELevelType MeshesLevelTypeInternal = ELevelType::None; //[G]

	/** Current attached mesh components. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "AttachedMeshes"))
	TArray<class UMeshComponent*> AttachedMeshesInternal; //[G]
};
