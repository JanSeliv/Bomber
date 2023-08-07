// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "UIDataAsset.generated.h"

enum class EEndGameState : uint8;

/**
 * Contains in-game UI data.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UUIDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the UI data asset. */
	static const UUIDataAsset& Get();

	/** Returns a class of the in-game widget.
	 * @see UUIDataAsset::InGameWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UInGameWidget> GetInGameWidgetClass() const { return InGameWidgetClassInternal; }

	/** Returns a class of the settings widget.
	 * @see UUIDataAsset::SettingsWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingsWidget> GetSettingsWidgetClass() const { return SettingsWidgetClassInternal; }

	/** Returns a class of the nickname widget.
	 * @see UUIDataAsset::NicknameWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UUserWidget> GetNicknameWidgetClass() const { return NicknameWidgetClassInternal; }

	/** Returns a class of the FPS counter widget.
	 * @see UUIDataAsset::FPSCounterWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UUserWidget> GetFPSCounterWidgetClass() const { return FPSCounterWidgetClassInternal; }

	/** Returns the localized texts about specified end game to display on UI.
	 * @see UUIDataAsset::EndGameTexts. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FText& GetEndGameText(EEndGameState EndGameState) const { return EndGameTextsInternal.FindChecked(EndGameState); }

protected:
	/** The class of a In-Game Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In-Game Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UInGameWidget> InGameWidgetClassInternal = nullptr;

	/** The class of a Settings Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingsWidget> SettingsWidgetClassInternal = nullptr;

	/** The class of a Nickname Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Nickname Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UUserWidget> NicknameWidgetClassInternal = nullptr;

	/** The class of a FPS counter widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "FPS Counter Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UUserWidget> FPSCounterWidgetClassInternal = nullptr;

	/** Contains the localized texts about specified end game to display on UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "End-Game Texts", ShowOnlyInnerProperties))
	TMap<EEndGameState, FText> EndGameTextsInternal;
};