// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/PlayerNameWidget.h"

// Bomber
#include "DataAssets/PlayerDataAsset.h"

// UE
#include "Components/Image.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerNameWidget)

/*********************************************************************************************
 * Player Name
 ********************************************************************************************* */

// Returns the player name from the widget
FText UPlayerNameWidget::GetPlayerName() const
{
	return PlayerNameTextWidget ? PlayerNameTextWidget->GetText() : FText::GetEmpty();
}

// Sets player name to the widget
void UPlayerNameWidget::SetPlayerName(const FText& NewPlayerName)
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
void UPlayerNameWidget::SetAssociatedPlayerId(int32 NewPlayerId)
{
	if (ensureMsgf(NewPlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'NewPlayer' is null!"), __LINE__, __FUNCTION__))
	{
		AssociatedPlayerIdInternal = NewPlayerId;
		SetBackgroundMaterial(NewPlayerId);
	}
}

// Sets the background material for the nameplate
void UPlayerNameWidget::SetBackgroundMaterial(int32 PlayerId)
{
	// Retrieve player-specific material configuration
	const UPlayerDataAsset& PlayerDataAsset = UPlayerDataAsset::Get();
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
void UPlayerNameWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetBackgroundMaterial(AssociatedPlayerIdInternal);
}