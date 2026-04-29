// Copyright (c) Yevhenii Selivanov

#include "Components/BmrSkeletalMeshComponent.h"

// Bomber
#include "Bomber.h"
#include "DalRegistrySubsystem.h"
#include "DataRegistries/BmrPlayerPropRow.h"
#include "DataRegistries/BmrPlayerRow.h"
#include "DataRegistries/BmrPlayerSkinRow.h"
#include "GameFramework/BmrPlayerState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "Animation/AnimSequence.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrSkeletalMeshComponent)

// Default constructor, overrides in object initializer default mesh by bomber mesh
ABmrSkeletalMeshActor::ABmrSkeletalMeshActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UBmrSkeletalMeshComponent>(TEXT("SkeletalMeshComponent0"))) // override default mesh class
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

#if WITH_EDITORONLY_DATA
	// Make this preview actor always loaded
	bIsSpatiallyLoaded = false;
#endif

	// Since it's preview mesh, make sure it is always visible even with close camera
	UBmrSkeletalMeshComponent& Mesh = GetMeshChecked();
	Mesh.BoundsScale = 6.f;
	Mesh.bNeverDistanceCull = true;
	Mesh.bAllowCullDistanceVolume = false;

	// Enable all lighting channels, so it's clearly visible in the dark
	Mesh.SetLightingChannels(/*bChannel0*/ true, /*bChannel1*/ true, /*bChannel2*/ true);
}

// Returns the Skeletal Mesh of bombers
UBmrSkeletalMeshComponent* ABmrSkeletalMeshActor::GetMeshComponent() const
{
	return Cast<UBmrSkeletalMeshComponent>(GetSkeletalMeshComponent());
}

UBmrSkeletalMeshComponent& ABmrSkeletalMeshActor::GetMeshChecked() const
{
	return *CastChecked<UBmrSkeletalMeshComponent>(GetSkeletalMeshComponent());
}

// Applies the specified player data by given type to the mesh
void ABmrSkeletalMeshActor::InitSkeletalMesh(const FBmrPlayerTag& InPlayerTag, int32 InSkinIndex)
{
	PlayerTag = InPlayerTag;
	SkinIndex = InSkinIndex;

	FBmrMeshData PlayerMeshData = FBmrMeshData::Empty;
	PlayerMeshData.RowName = FBmrPlayerRow::GetRowNameByPlayerTag(InPlayerTag);
	PlayerMeshData.SkinRowName = FBmrPlayerSkinRow::GetSkinRowName(InPlayerTag, InSkinIndex);
	GetMeshChecked().InitSkeletalMesh(PlayerMeshData);
}

// Called when an instance of this class is placed (in editor) or spawned
void ABmrSkeletalMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const bool bIsBlueprintViewport = UUtilsLibrary::IsEditor() && FTransientChecker::IsTransientLevel(this);
	if (!bIsBlueprintViewport && IS_TRANSIENT(this))
	{
		return;
	}

	InitSkeletalMesh(PlayerTag, SkinIndex);
}

// Called right before components are initialized, only called during gameplay
void ABmrSkeletalMeshActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register actor to let it to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// Sets default values for this component's properties
UBmrSkeletalMeshComponent::UBmrSkeletalMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CastShadow = false;
}

// Controls what kind of collision is enabled for this body and all attached props
void UBmrSkeletalMeshComponent::SetCollisionEnabled(ECollisionEnabled::Type NewType)
{
	Super::SetCollisionEnabled(NewType);

	for (UMeshComponent* AttachedMesh : AttachedMeshes)
	{
		if (AttachedMesh)
		{
			AttachedMesh->SetCollisionEnabled(NewType);
		}
	}
}

// Enables or disables gravity for the owner body and all attached meshes from the player row
void UBmrSkeletalMeshComponent::SetEnableGravity(bool bGravityEnabled)
{
	Super::SetEnableGravity(bGravityEnabled);

	for (UMeshComponent* MeshComponentIt : AttachedMeshes)
	{
		if (MeshComponentIt)
		{
			MeshComponentIt->SetEnableGravity(bGravityEnabled);
		}
	}
}

// Overridable internal function to respond to changes in the visibility of the component.
void UBmrSkeletalMeshComponent::OnVisibilityChanged()
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
void UBmrSkeletalMeshComponent::SetActive(bool bNewActive, bool bReset /*= false*/)
{
	Super::SetActive(bNewActive, bReset);

	// If anything activates or disables this preview actor, change its visibility as well
	constexpr bool bPropagateToChildren = false; // don't affect attached actors such as Camera
	SetHiddenInGame(!bNewActive, bPropagateToChildren);

	// Handle all attached props
	for (UMeshComponent* AttachedMeshIt : AttachedMeshes)
	{
		if (AttachedMeshIt)
		{
			AttachedMeshIt->SetActive(bNewActive, bReset);
			AttachedMeshIt->SetHiddenInGame(!bNewActive, bPropagateToChildren);
		}
	}
}

// Is overridden to properly apply the new mesh data
void UBmrSkeletalMeshComponent::SetSkeletalMesh(USkeletalMesh* NewMesh, bool bReinitPose)
{
	Super::SetSkeletalMesh(NewMesh, bReinitPose);

	if (!NewMesh)
	{
		Cleanup();
		return;
	}

	const APawn* OwnerPawn = GetOwner<APawn>();
	const ABmrPlayerState* OwnerPlayerState = OwnerPawn ? OwnerPawn->GetPlayerState<ABmrPlayerState>() : nullptr;
	const FBmrMeshData& NewMeshData = OwnerPlayerState ? OwnerPlayerState->GetChosenMeshData() : FBmrMeshData::Empty;
	if (NewMeshData != PlayerMeshData)
	{
		InitSkeletalMesh(NewMeshData);
	}
}

// Init this component by specified player data
void UBmrSkeletalMeshComponent::InitSkeletalMesh(const FBmrMeshData& MeshData)
{
	if (!MeshData.IsValid())
	{
		return;
	}

	// Resolve mesh from the Data Registry row
	const FBmrPlayerRow* PlayerRow = FBmrPlayerRow::GetRowByName(MeshData.RowName);
	USkeletalMesh* NewSkeletalMesh = PlayerRow ? Cast<USkeletalMesh>(PlayerRow->Mesh.Get()) : nullptr;
	if (!NewSkeletalMesh)
	{
		return;
	}

	if (!IsRegistered())
	{
		// If component is not registered, then register it to be able to attach props
		RegisterComponent();
	}

	// Set the new mesh data first, so it will not recursively call this function
	PlayerMeshData = MeshData;

	SetSkeletalMesh(NewSkeletalMesh, true);

	AttachProps();

	ApplySkinByRowName(MeshData.SkinRowName);
}

// No longer needed: skins are now separate material instances in FBmrPlayerSkinRow

// Returns the Player Tag to which this mesh is associated with
const FBmrPlayerTag& UBmrSkeletalMeshComponent::GetPlayerTag() const
{
	const FBmrPlayerRow* PlayerRow = FBmrPlayerRow::GetRowByName(PlayerMeshData.RowName);
	return PlayerRow ? PlayerRow->PlayerTag : FBmrPlayerTag::None;
}

// Gets all attached mesh components by specified filter class
void UBmrSkeletalMeshComponent::GetAttachedPropsByClass(TArray<UMeshComponent*>& OutMeshComponents, const TSubclassOf<class UMeshComponent>& FilterClass) const
{
	for (const TObjectPtr<UMeshComponent>& AttachedMeshIt : AttachedMeshes)
	{
		if (AttachedMeshIt
		    && AttachedMeshIt->IsA(FilterClass))
		{
			OutMeshComponents.Emplace(AttachedMeshIt);
		}
	}
}

// Attach all player props from Data Registry
void UBmrSkeletalMeshComponent::AttachProps()
{
	const FBmrPlayerRow* PlayerRow = FBmrPlayerRow::GetRowByName(PlayerMeshData.RowName);
	if (!PlayerRow
	    || !ArePropsWantToUpdate())
	{
		return;
	}

	const FBmrPlayerTag& PlayerTag = PlayerRow->PlayerTag;

	// Destroy previous meshes
	DetachProps();

	// Gather all prop rows with names for this player tag
	TArray<TPair<FName, const FBmrPlayerPropRow*>> NamedPropRows;
	FBmrPlayerPropRow::ForEachRowWithName([&PlayerTag, &NamedPropRows](FName RowName, const FBmrPlayerPropRow& Row)
	{
		if (Row.PlayerTag == PlayerTag)
		{
			NamedPropRows.Emplace(RowName, &Row);
		}
	});

	// Spawn new components and attach meshes
	for (const TPair<FName, const FBmrPlayerPropRow*>& NamedRow : NamedPropRows)
	{
		const FName PropRowName = NamedRow.Key;
		const FBmrPlayerPropRow* PropRow = NamedRow.Value;
		if (!ensureMsgf(PropRowName.IsValid(), TEXT("ASSERT: [%i] %hs:\n'PropRowName' is not valid for player tag '%s'!"), __LINE__, __FUNCTION__, *PlayerTag.ToString())
		    || !ensureMsgf(PropRow, TEXT("ASSERT: [%i] %hs:\n'PropAsset' is not valid for player tag '%s'!"), __LINE__, __FUNCTION__, *PlayerTag.ToString())
		    || !ensureMsgf(PropRow->Socket.IsValid(), TEXT("ASSERT: [%i] %hs:\n'Socket' is None for prop '%s'!"), __LINE__, __FUNCTION__, *PropRowName.ToString())
		    || !ensureMsgf(!PropRow->Mesh.IsNull(), TEXT("ASSERT: [%i] %hs:\n'Mesh' is not set for prop '%s'!"), __LINE__, __FUNCTION__, *PropRowName.ToString()))
		{
			return;
		}

		// Defer this prop if mesh not loaded yet, wait for DAL to signal availability
		UStreamableRenderAsset* PropAsset = PropRow->Mesh.Get();
		if (!PropAsset)
		{
			UDalRegistrySubsystem::Get().ListenForDataRegistryRow<FBmrPlayerPropRow>(this, PropRowName, [this](const FBmrPlayerPropRow&)
			{
				AttachProps();
			});
			continue;
		}

		UAnimSequence* PropAnim = PropRow->MeshAnimation.Get();

		UMeshComponent* MeshComponent = nullptr;
		if (USkeletalMesh* SkeletalMeshProp = Cast<USkeletalMesh>(PropAsset))
		{
			USkeletalMeshComponent* SkeletalComponent = NewObject<USkeletalMeshComponent>(this);
			SkeletalComponent->SetSkeletalMesh(SkeletalMeshProp);
			SkeletalComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			if (PropAnim)
			{
				SkeletalComponent->OverrideAnimationData(PropAnim);
			}
			MeshComponent = SkeletalComponent;
		}
		else if (UStaticMesh* StaticMeshProp = Cast<UStaticMesh>(PropAsset))
		{
			UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this);
			StaticMeshComponent->SetStaticMesh(StaticMeshProp);
			StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			MeshComponent = StaticMeshComponent;
		}

		if (!ensureMsgf(MeshComponent, TEXT("'MeshComponent' can not be attached with mesh '%s'"), *GetNameSafe(PropAsset)))
		{
			continue;
		}

		// Repeat the tweaks
		MeshComponent->SetCastShadow(CastShadow);
		MeshComponent->LightingChannels = LightingChannels;

		AttachedMeshes.Emplace(MeshComponent);
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
		MeshComponent->AttachToComponent(this, AttachRules, PropRow->Socket);
		MeshComponent->ClearFlags(RF_Transient);
	}
}

// Destroyed all currently equipped props
void UBmrSkeletalMeshComponent::DetachProps()
{
	for (int32 Index = AttachedMeshes.Num() - 1; Index >= 0; --Index)
	{
		UMeshComponent* MeshComponentIt = AttachedMeshes.IsValidIndex(Index) ? AttachedMeshes[Index] : nullptr;
		if (MeshComponentIt)
		{
			AttachedMeshes.RemoveAt(Index);
			MeshComponentIt->DestroyComponent();
		}
	}
}

// Returns true when is needed to attach or detach props
bool UBmrSkeletalMeshComponent::ArePropsWantToUpdate() const
{
	const FBmrPlayerTag& PlayerTag = GetPlayerTag();
	if (!PlayerTag.IsValid())
	{
		return false;
	}

	// Gather expected props from DR
	TArray<const FBmrPlayerPropRow*> PropRows;
	FBmrPlayerPropRow::GetPlayerProps(PlayerTag, PropRows);

	if (PropRows.IsEmpty())
	{
		// Returns true to detach when props list is empty but something is already attached
		return !AttachedMeshes.IsEmpty();
	}

	for (const FBmrPlayerPropRow* PropRow : PropRows)
	{
		UStreamableRenderAsset* PropAsset = PropRow->Mesh.Get();
		if (!PropAsset)
		{
			// Soft reference not resolved yet, props are not ready for update
			return false;
		}

		const bool bContains = AttachedMeshes.ContainsByPredicate([PropAsset](const UMeshComponent* MeshCompIt)
		{
			if (const USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(MeshCompIt))
			{
				return SkeletalMeshComp->GetSkinnedAsset() == PropAsset;
			}
			if (const UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(MeshCompIt))
			{
				return StaticMeshComp->GetStaticMesh() == PropAsset;
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

// Completely clears component
void UBmrSkeletalMeshComponent::Cleanup()
{
	if (!PlayerMeshData.IsValid())
	{
		// Is already cleaned up
		return;
	}

	DetachProps();

	PlayerMeshData = FBmrMeshData::Empty;
	SetSkeletalMesh(nullptr);
}

/*********************************************************************************************
 * Skins
 ********************************************************************************************* */

// Returns the total number of skins for current mesh from FBmrPlayerSkinRow
int32 UBmrSkeletalMeshComponent::GetSkinTexturesNum() const
{
	return FBmrPlayerSkinRow::GetSkinTexturesNum(GetPlayerTag());
}

// Returns the positional index of the currently applied skin among skins for the same PlayerTag
int32 UBmrSkeletalMeshComponent::GetAppliedSkinIndex() const
{
	const FBmrPlayerTag& PlayerTag = GetPlayerTag();
	int32 MatchIndex = 0;
	int32 FoundIndex = 0;
	FBmrPlayerSkinRow::ForEachRowWithName([&](FName ItemName, const FBmrPlayerSkinRow& SkinRow)
	{
		if (SkinRow.PlayerTag != PlayerTag)
		{
			return;
		}
		if (ItemName == PlayerMeshData.SkinRowName)
		{
			FoundIndex = MatchIndex;
		}
		++MatchIndex;
	});
	return FoundIndex;
}

// Checks if a skin is available and can be applied by index
bool UBmrSkeletalMeshComponent::IsSkinAvailable(int32 SkinIdx) const
{
	// Check if the corresponding skin bit is set (available), e.g: 0101 -> Only first and third skins are available
	return (PlayerMeshData.SkinAvailabilityMask & (1 << SkinIdx)) != 0;
}

// Makes skin unavailable or allows to apply by index
void UBmrSkeletalMeshComponent::SetSkinAvailable(bool bMakeAvailable, int32 SkinIdx)
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
		PlayerMeshData.SkinAvailabilityMask |= (1 << SkinIdx);
	}
	else
	{
		// E.g: player currently has all skins available: 1111
		// if call first SetSkinAvailable(false, 3), it clears the 1000 bit, where skin index #3 will become unavailable
		// then, if call SetSkinAvailable(false, 2), it also clears the 0100 bit, where skin index #2 will become unavailable as well
		// In result, the mask will be 0011, where only skins #0 and #1 remain available
		PlayerMeshData.SkinAvailabilityMask &= ~(1 << SkinIdx);
	}
}

// Set and apply new skin for current mesh by FBmrPlayerSkinRow row name
void UBmrSkeletalMeshComponent::ApplySkinByRowName(FName InSkinRowName)
{
	const FBmrPlayerSkinRow* SkinRow = InSkinRowName.IsNone() ? nullptr : FBmrPlayerSkinRow::GetRowByName(InSkinRowName);
	UMaterialInterface* SkinMaterial = SkinRow ? SkinRow->Material.Get() : nullptr;

	auto SetMaterialForAllSlots = [SkinMaterial](UMeshComponent* MeshComponent)
	{
		if (!MeshComponent)
		{
			return;
		}

		const TArray<UMaterialInterface*>& AllMaterials = MeshComponent->GetMaterials();
		for (int32 Index = 0; Index < AllMaterials.Num(); ++Index)
		{
			MeshComponent->SetMaterial(Index, SkinMaterial);
		}
	};

	// Set skin for own skeletal mesh and all attached props
	SetMaterialForAllSlots(this);
	for (const TObjectPtr<UMeshComponent>& AttachedMeshIt : AttachedMeshes)
	{
		SetMaterialForAllSlots(AttachedMeshIt);
	}

	PlayerMeshData.SkinRowName = InSkinRowName;
}

// Set and apply new skin for current mesh by positional index among skins for the same PlayerTag
void UBmrSkeletalMeshComponent::ApplySkinByIndex(int32 SkinIndex)
{
	ApplySkinByRowName(FBmrPlayerSkinRow::GetSkinRowName(GetPlayerTag(), SkinIndex));
}