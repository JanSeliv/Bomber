// Copyright (c) Yevhenii Selivanov

#include "Movement/BmrMoverWalkingMode.h"

// Bomber
#include "Structures/BmrMoverSyncState.h"

// UE
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "MoverComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMoverWalkingMode)

// Called when the mode is registered, initializes cached settings
void UBmrMoverWalkingMode::OnRegistered(const FName ModeName)
{
	Super::OnRegistered(ModeName);

	const UMoverComponent* MoverComponent = GetMoverComponent();
	checkf(MoverComponent, TEXT("ERROR: [%i] %hs:\n'MoverComponent' is null!"), __LINE__, __FUNCTION__);
	MutableLegacySettings = MoverComponent->FindSharedSettings_Mutable<UCommonLegacyMovementSettings>();
	checkf(MutableLegacySettings, TEXT("ERROR: [%i] %hs:\n'CachedCommonSettings' is null!"), __LINE__, __FUNCTION__);
	CachedMaxSpeed = MutableLegacySettings->MaxSpeed;
}

// Is overridden to handle walking-related movement
void UBmrMoverWalkingMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	// We need to modify MaxSpeed before calling Super because:
	// 1. Base class uses UCommonLegacyMovementSettings::MaxSpeed in multiple places (Params.MaxSpeed, friction logic, etc.)
	// 2. FGroundMoveParams doesn't allow selective parameter override after construction
	// 3. Alternative would require copying entire super method just to change one value
	// 4. Temporary modification preserves all base class logic and future updates
	const FBmrMoverSyncState* PowerupsState = StartState.SyncState.SyncStateCollection.FindDataByType<FBmrMoverSyncState>();
	const float BonusSpeed = PowerupsState ? PowerupsState->SkatePowerupAttribute * SkateSpeedBonus : 0.f;
	checkf(MutableLegacySettings, TEXT("ERROR: [%i] %hs:\n'MutableLegacySettings' is null!"), __LINE__, __FUNCTION__);
	const float OriginalMaxSpeed = MutableLegacySettings->MaxSpeed;
	MutableLegacySettings->MaxSpeed += BonusSpeed;

	Super::GenerateMove_Implementation(StartState, TimeStep, OutProposedMove);

	// Restore original max speed after Super call
	MutableLegacySettings->MaxSpeed = OriginalMaxSpeed;
}