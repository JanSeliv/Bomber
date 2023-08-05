// Copyright (c) Yevhenii Selivanov

#include "Components/MySkeletalMeshComponent.h"
//---
#include "DataAssets/PlayerDataAsset.h"
//---
#include "Animation/AnimSequence.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MySkeletalMeshComponent)

// Constructor that initializes the player data by specified tag
FCustomPlayerMeshData::FCustomPlayerMeshData(const FPlayerTag& PlayerTag, int32 InSkinIndex)
{
	PlayerRow = UPlayerDataAsset::Get().GetRowByPlayerTag(PlayerTag);
	SkinIndex = InSkinIndex;
}

// Constructor that initializes the data directly
FCustomPlayerMeshData::FCustomPlayerMeshData(const UPlayerRow& InPlayerRow, int32 InSkinIndex)
	: PlayerRow(&InPlayerRow)
	, SkinIndex(InSkinIndex) {}

// Default constructor, overrides in object initializer default mesh by bomber mesh
AMySkeletalMeshActor::AMySkeletalMeshActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMySkeletalMeshComponent>(TEXT("SkeletalMeshComponent0"))) // override default mesh class
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Returns the Skeletal Mesh of bombers
UMySkeletalMeshComponent* AMySkeletalMeshActor::GetMySkeletalMeshComponent() const
{
	return Cast<UMySkeletalMeshComponent>(GetSkeletalMeshComponent());
}

UMySkeletalMeshComponent& AMySkeletalMeshActor::GetMeshChecked() const
{
	return *CastChecked<UMySkeletalMeshComponent>(GetSkeletalMeshComponent());
}

// Applies the specified player data by given type to the mesh
void AMySkeletalMeshActor::InitMySkeletalMesh(const FPlayerTag& InPlayerTag, int32 InSkinIndex)
{
	PlayerTagInternal = InPlayerTag;
	SkinIndexInternal = InSkinIndex;

	const FCustomPlayerMeshData PlayerMeshData(InPlayerTag, InSkinIndex);
	GetMeshChecked().InitMySkeletalMesh(PlayerMeshData);
}

// Called when an instance of this class is placed (in editor) or spawned
void AMySkeletalMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this))
	{
		return;
	}

	InitMySkeletalMesh(PlayerTagInternal, SkinIndexInternal);
}

// Called right before components are initialized, only called during gameplay
void AMySkeletalMeshActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register actor to let it to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// Sets default values for this component's properties
UMySkeletalMeshComponent::UMySkeletalMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CastShadow = false;
}

// Controls what kind of collision is enabled for this body and all attached props
void UMySkeletalMeshComponent::SetCollisionEnabled(ECollisionEnabled::Type NewType)
{
	Super::SetCollisionEnabled(NewType);

	for (UMeshComponent* AttachedMesh : AttachedMeshesInternal)
	{
		if (AttachedMesh)
		{
			AttachedMesh->SetCollisionEnabled(NewType);
		}
	}
}

// Enables or disables gravity for the owner body and all attached meshes from the player row
void UMySkeletalMeshComponent::SetEnableGravity(bool bGravityEnabled)
{
	Super::SetEnableGravity(bGravityEnabled);

	for (UMeshComponent* MeshComponentIt : AttachedMeshesInternal)
	{
		if (MeshComponentIt)
		{
			MeshComponentIt->SetEnableGravity(bGravityEnabled);
		}
	}
}

// Overridable internal function to respond to changes in the visibility of the component.
void UMySkeletalMeshComponent::OnVisibilityChanged()
{
	Super::OnVisibilityChanged();

	const bool bIsVisible = IsVisible();

	const bool bIsAnimationAssetMode = GetSingleNodeInstance() != nullptr;
	if (bIsAnimationAssetMode)
	{
		if (bIsVisible)
		{
			Play(true);
		}
		else
		{
			Stop();
		}
	}
}

// Init this component by specified player data
void UMySkeletalMeshComponent::InitMySkeletalMesh(const FCustomPlayerMeshData& CustomPlayerMeshData)
{
	if (!CustomPlayerMeshData.PlayerRow)
	{
		return;
	}

	if (!IsRegistered())
	{
		// If component is not registered, then register it to be able to attach props
		RegisterComponent();
	}

	PlayerMeshDataInternal = CustomPlayerMeshData;

	USkeletalMesh* NewSkeletalMesh = Cast<USkeletalMesh>(CustomPlayerMeshData.PlayerRow->Mesh);
	SetSkeletalMesh(NewSkeletalMesh, true);

	AttachProps();

	SetSkin(CustomPlayerMeshData.SkinIndex);
}

// Returns level type to which this mesh is associated with
ELevelType UMySkeletalMeshComponent::GetAssociatedLevelType() const
{
	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	return PlayerRow ? PlayerRow->LevelType : ELevelType::None;
}

// Returns the Player Tag to which this mesh is associated with
const FPlayerTag& UMySkeletalMeshComponent::GetPlayerTag() const
{
	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	return PlayerRow ? PlayerRow->PlayerTag : FPlayerTag::None;
}

// Gets all attached mesh components by specified filter class
void UMySkeletalMeshComponent::GetAttachedPropsByClass(TArray<UMeshComponent*>& OutMeshComponents, const TSubclassOf<class UMeshComponent>& FilterClass) const
{
	for (const TObjectPtr<UMeshComponent>& AttachedMeshIt : AttachedMeshesInternal)
	{
		if (AttachedMeshIt
		    && AttachedMeshIt->IsA(FilterClass))
		{
			OutMeshComponents.Emplace(AttachedMeshIt);
		}
	}
}

// Attach all FAttachedMeshes to specified parent mesh
void UMySkeletalMeshComponent::AttachProps()
{
	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	if (!PlayerRow
	    || !ArePropsWantToUpdate())
	{
		return;
	}

	AttachedMeshesTypeInternal = PlayerRow->LevelType;

	// Destroy previous meshes
	for (int32 Index = AttachedMeshesInternal.Num() - 1; Index >= 0; --Index)
	{
		UMeshComponent* MeshComponentIt = AttachedMeshesInternal.IsValidIndex(Index) ? AttachedMeshesInternal[Index] : nullptr;
		if (MeshComponentIt)
		{
			AttachedMeshesInternal.RemoveAt(Index);
			MeshComponentIt->DestroyComponent();
		}
	}

	// Spawn new components and attach meshes
	const TArray<FAttachedMesh>& PlayerProps = PlayerRow->PlayerProps;
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

		if (!ensureMsgf(MeshComponent, TEXT("'MeshComponent' can not be attached with mesh '%s'"), *GetNameSafe(AttachedMeshIt.AttachedMesh)))
		{
			continue;
		}

		// Repeat the tweaks
		MeshComponent->SetCastShadow(CastShadow);
		MeshComponent->LightingChannels = LightingChannels;

		// Attach the prop: location is 0, rotation is parent's world, scale is 1
		AttachedMeshesInternal.Emplace(MeshComponent);
		MeshComponent->SetupAttachment(GetAttachmentRoot());
		MeshComponent->SetWorldTransform(GetComponentTransform());
		const FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld,
			EAttachmentRule::SnapToTarget,
			true);
		MeshComponent->AttachToComponent(this, AttachRules, AttachedMeshIt.Socket);
		MeshComponent->RegisterComponent();
	}
}

// Returns true when is needed to attach or detach props
bool UMySkeletalMeshComponent::ArePropsWantToUpdate() const
{
	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	if (!PlayerRow)
	{
		return false;
	}

	const TArray<FAttachedMesh>& PlayerProps = PlayerRow->PlayerProps;
	const bool bEmptyPropsList = PlayerProps.Num() == 0;
	if (bEmptyPropsList)
	{
		// Returns false to do not update when props list is empty and is nothing already attached
		const bool bAttachedOutdated = AttachedMeshesInternal.Num() > 0;
		return bAttachedOutdated;
	}

	for (const FAttachedMesh& AttachedMeshIt : PlayerProps)
	{
		const bool bContains = AttachedMeshesInternal.ContainsByPredicate([&AttachedMeshIt](const UMeshComponent* MeshCompIt)
		{
			UMeshComponent* MeshComponent = nullptr;
			if (const auto SkeletalMeshComp = Cast<USkeletalMeshComponent>(MeshCompIt))
			{
				return SkeletalMeshComp->GetSkinnedAsset() == AttachedMeshIt.AttachedMesh;
			}
			else if (const auto StaticMeshComp = Cast<UStaticMeshComponent>(MeshCompIt))
			{
				return StaticMeshComp->GetStaticMesh() == AttachedMeshIt.AttachedMesh;
			}
			return false;
		});

		if (!bContains)
		{
			return true;
		}
	}

	return false;
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
		for (int32 Index = 0; Index < AllMaterials.Num(); ++Index)
		{
			MeshComponent->SetMaterial(Index, MaterialInstanceDynamic);
		}
	};

	// Set skin for own skeletal mesh and all attached props
	SetMaterialForAllSlots(this);
	for (const TObjectPtr<UMeshComponent>& AttachedMeshIt : AttachedMeshesInternal)
	{
		SetMaterialForAllSlots(AttachedMeshIt);
	}

	PlayerMeshDataInternal.SkinIndex = SkinIndex;
}
