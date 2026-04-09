// Copyright (c) Yevhenii Selivanov

#include "Widgets/NMMCinematicStateWidget.h"

// NMM
#include "Data/NMMDataAsset.h"
#include "Data/NmmStateTag.h"
#include "NmmGameplayTags.h"

// Bomber
#include "Controllers/BmrPlayerController.h"
#include "Subsystems/BmrWidgetsSubsystem.h"
#include "Subsystems/GlobalMessageSubsystem.h"

// UE
#include "Components/Button.h"
#include "Components/RadialSlider.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCinematicStateWidget)

// Applies the given time to hold the skip progress to skip the cinematic
void UNMMCinematicStateWidget::SetCurrentHoldTime(float NewHoldTime)
{
	CurrentHoldTime = NewHoldTime;

	const float MaxHoldTime = UNMMDataAsset::Get().GetSkipCinematicHoldTime();
	const float HoldProgressNormalized = FMath::Clamp(CurrentHoldTime / MaxHoldTime, 0.f, 1.f);

	checkf(SkipHoldProgress, TEXT("ERROR: [%i] %hs:\n'SkipHoldProgress' is null!"), __LINE__, __FUNCTION__);
	SkipHoldProgress->SetValue(HoldProgressNormalized);

	if (CurrentHoldTime >= MaxHoldTime)
	{
		OnCinematicSkipFinished();
	}
}

// Reset to default state
void UNMMCinematicStateWidget::ResetWidget()
{
	SetCurrentHoldTime(0.f);
}

// // Called after the underlying slate widget is constructed
void UNMMCinematicStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide this widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(NmmGameplayTags::Event::MenuStateChanged, this, &ThisClass::OnNewMainMenuStateChanged);

	if (SkipCinematicButton)
	{
		SkipCinematicButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		SkipCinematicButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCinematicSkipFinished);
	}
}

// Called when the widget is removed from the viewport
void UNMMCinematicStateWidget::NativeDestruct()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	// Clear cached CinematicSkipped so late-binding listeners receive fresh data on next menu load
	UGlobalMessageSubsystem::ClearCachedMessages(NmmGameplayTags::Event::CinematicSkipped, this);

	Super::NativeDestruct();
}

// Called when the Main Menu state was changed
void UNMMCinematicStateWidget::OnNewMainMenuStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const FNmmStateTag NewState(Payload.InstigatorTags.First());
	const bool bIsCinematic = NewState == FNmmStateTag::Cinematic;

	// Was in cinematic if this widget is currently visible
	const bool bWasCinematic = GetVisibility() != ESlateVisibility::Collapsed;

	if (bIsCinematic || bWasCinematic)
	{
		// Hide all other widgets in Cinematic state and display them back when left
		UBmrWidgetsSubsystem::Get().SetAllWidgetsVisibility(!bIsCinematic);
	}

	// Show this widget in Cinematic state
	SetVisibility(bIsCinematic ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	ResetWidget();
}

/*********************************************************************************************
 * Inputs
 ********************************************************************************************* */

// Is calling while the skip holding button is ongoing
void UNMMCinematicStateWidget::OnCinematicSkipOngoing_Implementation()
{
	const UWorld* World = GetWorld();
	checkf(World, TEXT("ERROR: [%i] %hs:\n'World' is null!"), __LINE__, __FUNCTION__);
	const float NewHoldTime = CurrentHoldTime + World->GetDeltaSeconds();

	SetCurrentHoldTime(NewHoldTime);
}

// Is called on skip cinematic button released (cancelled)
void UNMMCinematicStateWidget::OnCinematicSkipReleased_Implementation()
{
	ResetWidget();
}

// Is called to skip cinematic on finished holding the skip button or clicked on UI
void UNMMCinematicStateWidget::OnCinematicSkipFinished_Implementation()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	if (!MyPC)
	{
		return;
	}

	// Notify that user skipped the pre-game cinematic manually
	FGameplayEventData EventData;
	EventData.EventTag = NmmGameplayTags::Event::CinematicSkipped;
	EventData.Instigator = MyPC->GetPawn();
	UGlobalMessageSubsystem::BroadcastGlobalMessage(EventData);
	if (!MyPC->HasAuthority())
	{
		MyPC->ServerBroadcastMessage(EventData);
	}
}
