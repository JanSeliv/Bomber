// Copyright (c) Yevhenii Selivanov

#include "Components/PasSurrounderVisualizerComponent.h"

// PAS
#include "Components/PasSurrounderLogicComponent.h"
#include "Data/PasDataAsset.h"
#include "PasBlueprintFunctionLibrary.h"

// Bomber
#include "DalSubsystem.h"
#include "Structures/BmrCell.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// UE
#include "Components/RectLightComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PasSurrounderVisualizerComponent)

UPasSurrounderVisualizerComponent::UPasSurrounderVisualizerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// When surrounder emits next predicted cells after each wall step
void UPasSurrounderVisualizerComponent::HandlePrediction_Implementation(const TArray<FPasCellDataOnSide>& PredictedCells)
{
	Super::HandlePrediction_Implementation(PredictedCells);
	if (PredictedCells.IsEmpty())
	{
		// Nothing to visualise this step
		return;
	}
	// Use first predicted entry directly so cell resolution stays in sync with surrounder side
	const FPasCellDataOnSide& FirstCell = PredictedCells[0];
	const FBmrCell CellToMove = UBmrCellUtilsLibrary::GetCellByPositionOnLevel(FirstCell.Column, FirstCell.Row);
	if (!CellToMove.IsValid())
	{
		// Cell lookup off grid, skip move
		return;
	}
	if (UBmrCellUtilsLibrary::IsCellExistsOnLevel(CellToMove))
	{
		Authority_MoveVisualizer(CellToMove);
	}
}

// When pre-spawn callback fires before first wall
void UPasSurrounderVisualizerComponent::OnBeforeFirstWallSpawn_Implementation()
{
	Super::OnBeforeFirstWallSpawn_Implementation();
	if (const AActor* MyOwner = GetOwner(); !MyOwner || !MyOwner->HasAuthority())
	{
		// Client-side, FlickerTime drives via OnRep
		return;
	}
	FlickerTime = UPasBlueprintFunctionLibrary::CalcFlickerTime(0);
	TriggerFlickerTimer();
}

// Authority-only: compute new replicated location from next cell and apply locally
void UPasSurrounderVisualizerComponent::Authority_MoveVisualizer(const FBmrCell& OnCell)
{
	if (const AActor* MyOwner = GetOwner(); !MyOwner || !MyOwner->HasAuthority())
	{
		// Client-side, VisualizerLocation drives via OnRep
		return;
	}
	// Cell location with configurable height offset so light sits above floor
	static constexpr float PercentDivisor = 100.f;
	FVector NewLoc = OnCell.Location;
	NewLoc.Z += UBmrCellUtilsLibrary::GetCellSize() * (static_cast<float>(UPasDataAsset::Get().VisualizerData.HeightPercent) / PercentDivisor);
	VisualizerLocation = NewLoc;
	UpdateVisualizerLocation_Internal();
}

// Builds (or rebuilds) RectLight and reapplies replicated state
void UPasSurrounderVisualizerComponent::CreateVisualiser()
{
	AActor* MyOwner = GetOwner();
	if (!ensureMsgf(MyOwner, TEXT("ASSERT: [%i] %hs:\n'MyOwner' condition is FALSE"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (!LightComponent)
	{
		// SetupAttachment + SetRelativeTransform before RegisterComponent so component
		// registers with parent already set and transform applied, avoiding brief
		// at-origin window before reparent
		LightComponent = NewObject<URectLightComponent>(MyOwner, TEXT("PasVisualizerLight"));
		LightComponent->SetupAttachment(MyOwner->GetRootComponent());
		// RelativeTransform_Rotation pitch -90 aims rect's +Y emission down at floor,
		// yaw/roll 180 orient width/height axes to align with cell grid
		static const FRotator LightRelativeRotation(-90.f, 180.f, 180.f);
		LightComponent->SetRelativeTransform(FTransform(LightRelativeRotation, FVector::ZeroVector, FVector::OneVector));
		LightComponent->RegisterComponent();
	}

	const FPasVisualizerData& VD = UPasDataAsset::Get().VisualizerData;
	const float CellSize = UBmrCellUtilsLibrary::GetCellSize();
	// Both SetSourceWidth and SetSourceHeight use WideOffset (square rect)
	// HeightPercent only drives Z offset + barn-door length, not rect source dims
	static constexpr float PercentToFraction = 0.01f;
	const float WideOffset = static_cast<float>(VD.WidePercent) * CellSize * PercentToFraction;
	const float HeightOffset = static_cast<float>(VD.HeightPercent) * CellSize * PercentToFraction;
	LightComponent->SetLightColor(VD.Color);
	LightComponent->SetSourceWidth(WideOffset);
	LightComponent->SetSourceHeight(WideOffset);
	LightComponent->SetIntensity(VD.Intensity);
	LightComponent->SetIntensityUnits(ELightUnits::Candelas);
	// Shape light into pencil-beam via barn doors: zero angle + length = HeightOffset * 2
	LightComponent->SetBarnDoorAngle(0.f);
	LightComponent->SetBarnDoorLength(HeightOffset * 2.f);
	LightComponent->SetCastShadows(false);
	LightComponent->SetHiddenInGame(true);
	// Race recovery: reapply replicated state landed before BeginPlay built light
	UpdateVisualizerLocation_Internal();
	TriggerFlickerTimer();
}

// Pushes replicated target location onto runtime light, null-safe when light not yet built
void UPasSurrounderVisualizerComponent::UpdateVisualizerLocation_Internal()
{
	if (!IsValid(LightComponent))
	{
		// Light not built yet (OnRep arrived before BeginPlay), CreateVisualiser reapply picks it up
		return;
	}
	if (VisualizerLocation.IsNearlyZero())
	{
		// Server has not authored target yet, nothing to apply
		return;
	}
	LightComponent->SetWorldLocation(VisualizerLocation, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);
}

// (Re)arms flicker timer using current replicated interval, null-safe when light not yet built
void UPasSurrounderVisualizerComponent::TriggerFlickerTimer()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		// World gone, cannot drive timer
		return;
	}
	World->GetTimerManager().ClearTimer(FlickerTimer);

	if (FlickerTime <= 0.f)
	{
		// Flicker is disabled while ticking, hide it
		if (LightComponent)
		{
			LightComponent->SetHiddenInGame(true);
		}
		return;
	}

	if (!IsValid(LightComponent))
	{
		// Light not built yet, CreateVisualiser reapply re-arms timer once light comes online
		return;
	}

	LightComponent->SetHiddenInGame(true);
	World->GetTimerManager().SetTimer(FlickerTimer,
	    FTimerDelegate::CreateUObject(this, &UPasSurrounderVisualizerComponent::FlickerTick),
	    static_cast<float>(FlickerTime), /*bLoop*/ true);
}

// Fires every flicker interval while surrounder is active
void UPasSurrounderVisualizerComponent::FlickerTick()
{
	if (!IsValid(LightComponent))
	{
		// Light was torn down between timer fires
		return;
	}
	LightComponent->SetHiddenInGame(!LightComponent->bHiddenInGame);
}

// Fires when surrounder transitions to next side
void UPasSurrounderVisualizerComponent::HandleSurrounderSideChanged(int32 PassedSidesNum)
{
	if (const AActor* MyOwner = GetOwner(); !MyOwner || !MyOwner->HasAuthority())
	{
		// Client-side, FlickerTime drives via OnRep
		return;
	}
	FlickerTime = UPasBlueprintFunctionLibrary::CalcFlickerTime(PassedSidesNum);
	TriggerFlickerTimer();
}

// Fires when server replicates updated VisualizerLocation to client
void UPasSurrounderVisualizerComponent::OnRep_VisualizerLocation()
{
	// Null-safe: UpdateVisualizerLocation_Internal short-circuits when light
	// not built. Either CreateVisualiser in BeginPlay reapplies, or later OnRep
	// arrives after light is built
	UpdateVisualizerLocation_Internal();
}

// Fires when server replicates updated FlickerTime to client
void UPasSurrounderVisualizerComponent::OnRep_FlickerTime()
{
	TriggerFlickerTimer();
}

void UPasSurrounderVisualizerComponent::BeginPlay()
{
	Super::BeginPlay();

	UDalSubsystem::Get().ListenForDataAsset<UPasDataAsset>(this, &ThisClass::OnDataAssetLoaded);

	if (Surrounder)
	{
		Surrounder->OnSideChanged.AddDynamic(this, &UPasSurrounderVisualizerComponent::HandleSurrounderSideChanged);
	}
}

// Fires once PlayAreaSurrounder data asset finishes async load via UDalSubsystem
void UPasSurrounderVisualizerComponent::OnDataAssetLoaded_Implementation(const UPasDataAsset* DataAsset)
{
	CreateVisualiser();
}

void UPasSurrounderVisualizerComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlickerTimer);
	}
	if (IsValid(LightComponent))
	{
		LightComponent->DestroyComponent();
		LightComponent = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void UPasSurrounderVisualizerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPasSurrounderVisualizerComponent, VisualizerLocation);
	DOREPLIFETIME(UPasSurrounderVisualizerComponent, FlickerTime);
}

// When surrounder resets, on round end or game restart
void UPasSurrounderVisualizerComponent::OnResetSurrounder_Implementation()
{
	Super::OnResetSurrounder_Implementation();
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlickerTimer);
	}
	if (IsValid(LightComponent))
	{
		LightComponent->SetHiddenInGame(true);
	}
	if (const AActor* MyOwner = GetOwner(); MyOwner && MyOwner->HasAuthority())
	{
		FlickerTime = 0.0;
	}
}
