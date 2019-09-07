// Copyright 2019 Yevhenii Selivanov.

#include "InGameWidget.h"

// Shows the in game menu.
void UInGameWidget::ShowInGameState_Implementation()
{
	// ...
}

// Called after the underlying slate widget is constructed. May be called multiple times due to adding and removing from the hierarchy.
void UInGameWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();
}

// Updates appearance dynamically in the editor
void UInGameWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}
