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
	if (!PlayerRow
	    || PlayerRow->LevelType == MeshesLevelTypeInternal)
	{
		return;
	}

	MeshesLevelTypeInternal = PlayerRow->LevelType;

	// Destroy previous meshes
	for (int32 i = AttachedMeshesInternal.Num() - 1; i >= 0; --i)
	{
		UMeshComponent* const& MeshComponentIt = AttachedMeshesInternal.IsValidIndex(i) ? AttachedMeshesInternal[i] : nullptr;
		if (MeshComponentIt)
		{
			AttachedMeshesInternal.RemoveAt(i);
			MeshComponentIt->DestroyComponent();
		}
	}

	// Spawn new components and attach meshes
	const TArray<FAttachedMesh>& PlayerProps = PlayerRow->PlayerProps;
	for (const FAttachedMesh& MeshIt : PlayerProps)
	{
		UMeshComponent* MeshComponent = nullptr;
		if (const auto SkeletalMeshProp = Cast<USkeletalMesh>(MeshIt.AttachedMesh))
		{
			USkeletalMeshComponent* SkeletalComponent = NewObject<USkeletalMeshComponent>(this);
			SkeletalComponent->SetSkeletalMesh(SkeletalMeshProp);
			SkeletalComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			MeshComponent = SkeletalComponent;
		}
		else if (const auto StaticMeshProp = Cast<UStaticMesh>(MeshIt.AttachedMesh))
		{
			UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this);
			StaticMeshComponent->SetStaticMesh(StaticMeshProp);
			StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			MeshComponent = StaticMeshComponent;
		}

		if (MeshComponent)
		{
			AttachedMeshesInternal.Add(MeshComponent);
			const FTransform Transform(GetRelativeRotation(), FVector::ZeroVector, GetRelativeScale3D());
			MeshComponent->SetupAttachment(GetAttachmentRoot());
			MeshComponent->SetRelativeTransform(Transform);
			const FAttachmentTransformRules AttachRules(
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepWorld,
				EAttachmentRule::SnapToTarget,
				true);
			MeshComponent->AttachToComponent(this, AttachRules, MeshIt.Socket);
			MeshComponent->RegisterComponent();
		}
	}
}

// Enables or disables gravity for the owner body and all attached meshes from the player row
void UMySkeletalMeshComponent::SetEnableGravity(bool bGravityEnabled)
{
	Super::SetEnableGravity(bGravityEnabled);

	for (UMeshComponent* const& MeshComponentIt : AttachedMeshesInternal)
	{
		MeshComponentIt->SetEnableGravity(bGravityEnabled);
	}
}
