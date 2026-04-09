// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/NmmMVVM_MenuViewModel.h"

// NMM
#include "Data/NmmStateTag.h"
#include "NmmGameplayTags.h"

// Bomber
#include "Subsystems/GlobalMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NmmMVVM_MenuViewModel)

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when this View Model is constructed
void UNmmMVVM_MenuViewModel::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(NmmGameplayTags::Event::MenuStateChanged, this, &ThisClass::OnMenuStateChanged);
}

// Is called when this View Model is destructed
void UNmmMVVM_MenuViewModel::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);
}

// Called when the Main Menu state was changed, updates own tag
void UNmmMVVM_MenuViewModel::OnMenuStateChanged_Implementation(const FGameplayEventData& Payload)
{
	SetCurrentMenuStateTag(FNmmStateTag(Payload.InstigatorTags.First()));
}
