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

#if WITH_EDITORONLY_DATA
	// Make this preview actor always loaded
	bIsSpatiallyLoaded = false;
#endif

	// Since it's preview mesh, make sure it is always visible even with close camera
	UMySkeletalMeshComponent& Mesh = GetMeshChecked();
	Mesh.BoundsScale = 6.f;
	Mesh.bNeverDistanceCull = true;
	Mesh.bAllowCullDistanceVolume = false;

	// Enable all lighting channels, so it's clearly visible in the dark
	Mesh.SetLightingChannels(/*bChannel0*/true, /*bChannel1*/true, /*bChannel2*/true);
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

// Disables tick and visibility if inactive and vice versa
void UMySkeletalMeshComponent::SetActive(bool bNewActive, bool bReset/*= false*/)
{
	Super::SetActive(bNewActive, bReset);

	// If anything activates or disables this preview actor, change its visibility as well
	constexpr bool bPropagateToChildren = false; // don't affect attached actors such as Camera
	SetHiddenInGame(!bNewActive, bPropagateToChildren);

	// Handle all attached props
	for (UMeshComponent* AttachedMeshIt : AttachedMeshesInternal)
	{
		if (AttachedMeshIt)
		{
			AttachedMeshIt->SetActive(bNewActive, bReset);
			AttachedMeshIt->SetHiddenInGame(!bNewActive, bPropagateToChildren);
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

	UpdateSkinTextures();

	AttachProps();

	ApplySkinByIndex(CustomPlayerMeshData.SkinIndex);
}

// Creates dynamic material instance for each skin if is not done before
void UMySkeletalMeshComponent::UpdateSkinTextures()
{
	if (ensureMsgf(PlayerMeshDataInternal.PlayerRow, TEXT("ASSERT: [%i] %hs:\n'PlayerRow' is null!"), __LINE__, __FUNCTION__))
	{
		const_cast<UPlayerRow*>(PlayerMeshDataInternal.PlayerRow.Get())->UpdateSkinTextures();
	}
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
		if (USkeletalMesh* SkeletalMeshProp = Cast<USkeletalMesh>(AttachedMeshIt.AttachedMesh))
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
		else if (UStaticMesh* StaticMeshProp = Cast<UStaticMesh>(AttachedMeshIt.AttachedMesh))
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

		AttachedMeshesInternal.Emplace(MeshComponent);
		MeshComponent->SetupAttachment(GetAttachmentRoot());
		MeshComponent->SetWorldTransform(GetComponentTransform());
		MeshComponent->RegisterComponent();

		// Attach the prop: location is 0, rotation is parent's world, scale is 1
		constexpr bool bInWeldSimulatedBodies = true;
		const FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld,
			EAttachmentRule::SnapToTarget,
			bInWeldSimulatedBodies);

		// Before attach, mark it as transient only for this operation to avoid Modify()-call that asks to save the level
		// However, remove the flag next as props have to be cooked
		MeshComponent->SetFlags(RF_Transient);
		MeshComponent->AttachToComponent(this, AttachRules, AttachedMeshIt.Socket);
		MeshComponent->ClearFlags(RF_Transient);
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
			if (const auto SkeletalMeshComp = Cast<USkeletalMeshComponent>(MeshCompIt))
			{
				return SkeletalMeshComp->GetSkinnedAsset() == AttachedMeshIt.AttachedMesh;
			}

			if (const auto StaticMeshComp = Cast<UStaticMeshComponent>(MeshCompIt))
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

/*********************************************************************************************
 * Skins
 ********************************************************************************************* */

// Returns the total number of skins for current mesh (player row)
int32 UMySkeletalMeshComponent::GetSkinTexturesNum() const
{
	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	return PlayerRow ? PlayerRow->GetSkinTexturesNum() : 0;
}

// Checks if a skin is available and can be applied by index
bool UMySkeletalMeshComponent::IsSkinAvailable(int32 SkinIdx) const
{
	// Check if the corresponding skin bit is set (available), e.g: 0101 -> Only first and third skins are available
	return (PlayerMeshDataInternal.SkinAvailabilityMask & (1 << SkinIdx)) != 0;
}

// Makes skin unavailable or allows to apply by index
void UMySkeletalMeshComponent::SetSkinAvailable(bool bMakeAvailable, int32 SkinIdx)
{
	if (IsSkinAvailable(SkinIdx) == bMakeAvailable)
	{
		// Is already set
		return;
	}

	constexpr int32 MaxSkinBits = 32;
	const int32 MaxSkinTextures = GetSkinTexturesNum();
	if (!ensureMsgf(FMath::IsWithin(SkinIdx, 0, MaxSkinBits), TEXT("ASSERT: [%i] %hs:\n'Attempted to set skin %i, but it is out of range [0, %i]"), __LINE__, __FUNCTION__, SkinIdx, MaxSkinBits)
	    || !ensureMsgf(SkinIdx < MaxSkinTextures, TEXT("ASSERT: [%i] %hs:\n'Attempted to set skin %i, which is larger than the total number of skins %i"), __LINE__, __FUNCTION__, SkinIdx, MaxSkinTextures))
	{
		return;
	}

	if (bMakeAvailable)
	{
		// E.g: player currently has none skins available: 0000
		// if call first SetSkinAvailable(true, 0), it will result in 0001, where skin index #0 will become available
		// then, if call SetSkinAvailable(true, 1), it will also add 0010, where skin index #1 will become available as well
		// In result, the mask will be 0011, where both skins #0 and #1 are available
		PlayerMeshDataInternal.SkinAvailabilityMask |= (1 << SkinIdx);
	}
	else
	{
		// E.g: player currently has all skins available: 1111
		// if call first SetSkinAvailable(false, 3), it clears the 1000 bit, where skin index #3 will become unavailable
		// then, if call SetSkinAvailable(false, 2), it also clears the 0100 bit, where skin index #2 will become unavailable as well
		// In result, the mask will be 0011, where only skins #0 and #1 remain available
		PlayerMeshDataInternal.SkinAvailabilityMask &= ~(1 << SkinIdx);
	}
}

// Set and apply new skin for current mesh, by index from player row
void UMySkeletalMeshComponent::ApplySkinByIndex(int32 SkinIndex)
{
	if (!ensureMsgf(PlayerMeshDataInternal.PlayerRow, TEXT("ASSERT: [%i] %hs:\n'PlayerMeshDataInternal.PlayerRow' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const UPlayerRow* PlayerRow = PlayerMeshDataInternal.PlayerRow;
	UMaterialInstanceDynamic* MaterialInstanceDynamic = PlayerRow->GetMaterialInstanceDynamic(SkinIndex);
	if (!ensureMsgf(MaterialInstanceDynamic, TEXT("ASSERT: ApplySkinByIndex: 'MaterialInstanceDynamic' is not valid for index %i"), SkinIndex))
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