// Copyright 2021 Yevhenii Selivanov

#include "Components/MySkeletalMeshComponent.h"
//---
#include "LevelActors/PlayerCharacter.h"

// Sets default values for this component's properties
UMySkeletalMeshComponent::UMySkeletalMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Attach all FAttachedMeshes to specified parent mesh
void UMySkeletalMeshComponent::AttachProps(const UPlayerRow* PlayerRow)
{
	if (!PlayerRow)
	{
		return;
	}

	const TArray<FAttachedMesh>& PlayerProps = PlayerRow->PlayerProps;
	for (const FAttachedMesh& MeshIt : PlayerProps)
	{
		UMeshComponent* MeshComponent = nullptr;
		if (const auto SkeletalMeshProp = Cast<USkeletalMesh>(MeshIt.AttachedMesh))
		{
			USkeletalMeshComponent* SkeletalComponent = NewObject<USkeletalMeshComponent>(this);
			SkeletalComponent->SetSkeletalMesh(SkeletalMeshProp);
			MeshComponent = SkeletalComponent;
		}
		else if (const auto StaticMeshProp = Cast<UStaticMesh>(MeshIt.AttachedMesh))
		{
			UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this);
			StaticMeshComponent->SetStaticMesh(StaticMeshProp);
			MeshComponent = StaticMeshComponent;
		}

		if (MeshComponent)
		{
			const FTransform Transform(GetRelativeRotation(), FVector::ZeroVector, GetRelativeScale3D());
			MeshComponent->SetupAttachment(GetAttachmentRoot());
			MeshComponent->SetRelativeTransform(Transform);
			MeshComponent->RegisterComponent();
			const FAttachmentTransformRules AttachRules(
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepWorld,
				EAttachmentRule::SnapToTarget,
				true);
			MeshComponent->AttachToComponent(this, AttachRules, MeshIt.Socket);
		}
	}
}
