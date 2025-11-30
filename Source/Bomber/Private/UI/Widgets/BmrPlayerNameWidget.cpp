// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/BmrPlayerNameWidget.h"

// Bomber
#include "DataAssets/BmrPlayerDataAsset.h"

// UE
#include "Components/Image.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerNameWidget)

/*********************************************************************************************
 * Player Name
 ********************************************************************************************* */

// Returns the player name from the widget
FText UBmrPlayerNameWidget::GetPlayerName() const
{
	return PlayerNameTextWidget ? PlayerNameTextWidget->GetText() : FText::GetEmpty();
}

// Sets player name to the widget
void UBmrPlayerNameWidget::SetPlayerName(const FText& NewPlayerName)
{
	checkf(PlayerNameTextWidget, TEXT("ERROR: [%i] %hs:\n'PlayerNameTextWidget' is null!"), __LINE__, __FUNCTION__);
	if (!PlayerNameTextWidget->GetText().IdenticalTo(NewPlayerName))
	{
		PlayerNameTextWidget->SetText(NewPlayerName);
	}
}

/*********************************************************************************************
 * Player ID
 ********************************************************************************************* */

// Sets the player character to the widget
void UBmrPlayerNameWidget::SetAssociatedPlayerId(int32 NewPlayerId)
{
	if (ensureMsgf(NewPlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'NewPlayer' is null!"), __LINE__, __FUNCTION__))
	{
		AssociatedPlayerId = NewPlayerId;
		SetBackgroundMaterial(NewPlayerId);
	}
}

// Sets the background material for the nameplate
void UBmrPlayerNameWidget::SetBackgroundMaterial(int32 PlayerId)
{
	// Retrieve player-specific material configuration
	const UBmrPlayerDataAsset& PlayerDataAsset = UBmrPlayerDataAsset::Get();
	const int32 NameplateMaterialsNum = PlayerDataAsset.GetNameplateMaterialsNum();

	if (NameplateMaterialsNum <= 0)
	{
		return;
	}

	// Calculate material index based on available materials
	const int32 MaterialIndex = PlayerId < NameplateMaterialsNum ? PlayerId : PlayerId % NameplateMaterialsNum;
	UMaterialInterface* BackgroundMaterial = PlayerDataAsset.GetNameplateMaterial(MaterialIndex);

	if (BackgroundMaterial)
	{
		checkf(BackgroundImageWidget, TEXT("ERROR: [%i] %hs:\n'BackgroundImage' is null!"), __LINE__, __FUNCTION__);
		BackgroundImageWidget->SetBrushFromMaterial(BackgroundMaterial);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called by both the game and the editor.  Allows users to run initial setup for their widgets to better preview
void UBmrPlayerNameWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetBackgroundMaterial(AssociatedPlayerId);
}