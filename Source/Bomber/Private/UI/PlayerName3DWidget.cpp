// Copyright (c) Yevhenii Selivanov

#include "UI/PlayerName3DWidget.h"
//---
#include "Components/TextBlock.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerName3DWidget)

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