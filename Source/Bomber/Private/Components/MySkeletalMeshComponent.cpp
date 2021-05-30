// Copyright 2021 Yevhenii Selivanov

#include "Components/MySkeletalMeshComponent.h"
//---
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
//---
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
UMySkeletalMeshComponent::UMySkeletalMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Change the SkeletalMesh that is rendered for this Component
void UMySkeletalMeshComponent::SetSkeletalMesh(USkeletalMesh* NewMesh, bool bReinitPose)
{
	Super::SetSkeletalMesh(NewMesh, bReinitPose);
}

// Changes the material applied to an element of the mesh
void UMySkeletalMeshComponent::SetMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	Super::SetMaterial(ElementIndex, Material);
}

// Enables or disables gravity for the owner body and all attached meshes from the player row
void UMySkeletalMeshComponent::SetEnableGravity(bool bGravityEnabled)
{
	Super::SetEnableGravity(bGravityEnabled);

	for (UMeshComponent* MeshComponentIt : AttachedMeshesInternal)
	{
		MeshComponentIt->SetEnableGravity(bGravityEnabled);
	}
}

// Overridable internal function to respond to changes in the visibility of the component.
void UMySkeletalMeshComponent::OnVisibilityChanged()
{
	Super::OnVisibilityChanged();

	if (IsVisible())
	{
		Play(true);
	}
	else
	{
		Stop();
	}
}

// Init this component by specified player data
void UMySkeletalMeshComponent::InitMySkeletalMesh(const FCustomPlayerMeshData& CustomPlayerMeshData)
{
	if (!CustomPlayerMeshData.PlayerRow)
	{
		return;
	}

	PlayerMeshDataInternal = CustomPlayerMeshData;

	USkeletalMesh* NewSkeletalMesh = Cast<USkeletalMesh>(CustomPlayerMeshData.PlayerRow->Mesh);
	SetSkeletalMesh(NewSkeletalMesh, true);

	AttachProps();

	SetSkin(CustomPlayerMeshData.SkinIndex);
}

// Attach all FAttachedMeshes to specified parent mesh
void UMySkeletalMeshComponent::AttachProps()
{
	if (!PlayerMeshDataInternal.PlayerRow
	    || PlayerMeshDataInternal.PlayerRow->LevelType == AttachedMeshesTypeInternal)
	{
		return;
	}

	AttachedMeshesTypeInternal = PlayerMeshDataInternal.PlayerRow->LevelType;

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
	const TArray<FAttachedMesh>& PlayerProps = PlayerMeshDataInternal.PlayerRow->PlayerProps;
	for (const FAttachedMesh& AttachedMeshIt : PlayerProps)
	{
		UMeshComponent* MeshComponent = nullptr;
		if (const auto SkeletalMeshProp = Cast<USkeletalMesh>(AttachedMeshIt.AttachedMesh))
		{
			USkeletalMeshComponent* SkeletalComponent = NewObject<USkeletalMeshComponent>(this);
			SkeletalComponent->SetSkeletalMesh(SkeletalMeshProp);
			SkeletalComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			if (AttachedMeshIt.MeshAnimation)
			{
				SkeletalComponent->OverrideAnimationData(AttachedMeshIt.MeshAnimation);
			}
			MeshComponent = SkeletalComponent;
		}
		else if (const auto StaticMeshProp = Cast<UStaticMesh>(AttachedMeshIt.AttachedMesh))
		{
			UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this);
			StaticMeshComponent->SetStaticMesh(StaticMeshProp);
			StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
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
			MeshComponent->AttachToComponent(this, AttachRules, AttachedMeshIt.Socket);
			MeshComponent->RegisterComponent();
		}
	}
}

// Some bomber characters have more than 1 texture, it will change a player skin if possible
void UMySkeletalMeshComponent::SetSkin(int32 SkinIndex)
{
	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	const int32 SkinTexturesNum = PlayerRow ? PlayerRow->GetMaterialInstancesDynamicNum() : 0;
	if (!SkinTexturesNum)
	{
		return;
	}

	SkinIndex %= SkinTexturesNum;
	const auto MaterialInstanceDynamic = PlayerRow->GetMaterialInstanceDynamic(SkinIndex);
	if (!ensureMsgf(MaterialInstanceDynamic, TEXT("ASSERT: SetSkin: 'MaterialInstanceDynamic' is not valid contained by index %i"), SkinIndex))
	{
		return;
	}

	auto SetMaterialForAllSlots = [MaterialInstanceDynamic](UMeshComponent* MeshComponent)
	{
		if (!MeshComponent)
		{
			return;
		}

		const TArray<UMaterialInterface*>& AllMaterials = MeshComponent->GetMaterials();
		for (int32 i = 0; i < AllMaterials.Num(); ++i)
		{
			MeshComponent->SetMaterial(i, MaterialInstanceDynamic);
		}
	};

	// Set skin for own skeletal mesh and all attached props
	SetMaterialForAllSlots(this);
	for (UMeshComponent* const& AttachedMeshIt : AttachedMeshesInternal)
	{
		SetMaterialForAllSlots(AttachedMeshIt);
	}

	PlayerMeshDataInternal.SkinIndex = SkinIndex;
}

// Called when a component is registered (not loaded)
void UMySkeletalMeshComponent::OnRegister()
{
	Super::OnRegister();
}
