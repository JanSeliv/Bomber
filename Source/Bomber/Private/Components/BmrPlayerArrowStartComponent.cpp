// Copyright (c) Yevhenii Selivanov

#include "Components/BmrPlayerArrowStartComponent.h"

// Bomber
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"

// UE
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerArrowStartComponent)

// Sets default values for this component's properties
UBmrPlayerArrowStartComponent::UBmrPlayerArrowStartComponent()
{
	// Can tick to apply transform animation from curve table
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetVisibility(false);
	SetUsingAbsoluteRotation(true);
	SetGenerateOverlapEvents(false);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Controls arrow visibility, tick, and animation state
void UBmrPlayerArrowStartComponent::SetArrowEnabled(bool bShouldEnable)
{
	if (IsVisible() == bShouldEnable)
	{
		return;
	}

	if (bShouldEnable
	    && !CanEnableArrow())
	{
		// Cannot enable for remote pawns or non-pawn actors
		return;
	}

	StartTime = bShouldEnable ? GetWorld()->GetTimeSeconds() : 0.f;
	SetVisibility(bShouldEnable);
	SetComponentTickEnabled(bShouldEnable);
}

// Returns true if the arrow can be enabled based on current conditions
bool UBmrPlayerArrowStartComponent::CanEnableArrow() const
{
	const APawn* OwnerPawn = GetOwner<APawn>();
	return OwnerPawn
	       && OwnerPawn->IsPlayerControlled()
	       && OwnerPawn->IsLocallyControlled();
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when the game starts
void UBmrPlayerArrowStartComponent::BeginPlay()
{
	Super::BeginPlay();

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	APawn* OwnerPawn = GetOwner<APawn>();
	if (ensureMsgf(OwnerPawn, TEXT("ASSERT: [%i] %hs:\n'OwnerPawn' is not pawn!"), __LINE__, __FUNCTION__))
	{
		OwnerPawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::OnPawnControllerChanged);
	}
}

// Updates animation from curve table
void UBmrPlayerArrowStartComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Apply animation from curve table (when tick is enabled)
	const float SecondsSinceStart = FMath::Max(GetWorld()->GetTimeSeconds() - StartTime, 0.f);
	const bool bIsAnimationFinished = !UGameplayUtilsLibrary::ApplyComponentTransformFromCurveTable(this, AnimationCurveTable, SecondsSinceStart);
	if (bIsAnimationFinished)
	{
		SetArrowEnabled(false);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Manages component state based on game state and local control
void UBmrPlayerArrowStartComponent::OnGameStateChanged_Implementation(EBmrCurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECGS::GameStarting:
			SetArrowEnabled(true);
			break;

		case ECGS::Menu:
			SetArrowEnabled(false);
			break;

		default: break;
	}
}

// Called when pawn controller changes
void UBmrPlayerArrowStartComponent::OnPawnControllerChanged_Implementation(APawn* Pawn, AController* OldController, AController* NewController)
{
	const bool bIsGameStarting = ABmrGameState::GetCurrentGameState() == ECGS::GameStarting;
	const bool bShouldEnable = bIsGameStarting && CanEnableArrow();
	SetArrowEnabled(bShouldEnable);
}