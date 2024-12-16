// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/PlayerName3DWidget.h"
//---
#include "LevelActors/PlayerCharacter.h"
//---
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerName3DWidget)

// Is overridden to hide dependent 3D widget components along with this widget
void UPlayerName3DWidget::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);

	// Hide 3D widget components if this widget is hidden
	if (PlayerOwnerInternal)
	{
		constexpr bool bPropagateToChildren = true;
		const bool bMakeVisible = InVisibility == ESlateVisibility::Visible;

		UStaticMeshComponent* NameplateMesh = PlayerOwnerInternal->GetNameplateMesh();
		checkf(NameplateMesh, TEXT("ERROR: [%i] %hs:\n'NameplateMesh' is null!"), __LINE__, __FUNCTION__);
		NameplateMesh->SetVisibility(bMakeVisible, bPropagateToChildren);
	}
}

// Is overridden to perform cleanup
void UPlayerName3DWidget::BeginDestroy()
{
	Super::BeginDestroy();

	PlayerOwnerInternal = nullptr;
}

/*********************************************************************************************
 * Player Name
 ********************************************************************************************* */

// Returns the player name from the widget
FName UPlayerName3DWidget::GetPlayerName() const
{
	return PlayerNameTextWidget ? FName(*PlayerNameTextWidget->GetText().ToString()) : NAME_None;
}

// Sets player name to the widget
void UPlayerName3DWidget::SetPlayerName(FName NewPlayerName)
{
	checkf(PlayerNameTextWidget, TEXT("ERROR: [%i] %hs:\n'PlayerNameTextWidget' is null!"), __LINE__, __FUNCTION__);
	const FText PlayerNameText = FText::FromName(NewPlayerName);
	if (!PlayerNameTextWidget->GetText().IdenticalTo(PlayerNameText))
	{
		PlayerNameTextWidget->SetText(PlayerNameText);
	}
}

/*********************************************************************************************
 * Player Owner
 ********************************************************************************************* */

// Sets the player character to the widget
void UPlayerName3DWidget::SetPlayerOwner(APlayerCharacter* NewPlayer)
{
	if (ensureMsgf(NewPlayer, TEXT("ASSERT: [%i] %hs:\n'NewPlayer' is null!"), __LINE__, __FUNCTION__))
	{
		PlayerOwnerInternal = NewPlayer;
	}
}
