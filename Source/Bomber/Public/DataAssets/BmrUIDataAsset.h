// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// Bomber
#include "Structures/BmrPowerupTag.h"

#include "BmrUIDataAsset.generated.h"

enum class EBmrEndGameState : uint8;
enum class EBmrPlayerType : uint8;

/**
 * Contains configuration data for UI.
 * Widgets are stored in FBmrWidgetRow Data Registry rows
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrUIDataAsset final : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the UI data asset. */
	static const UBmrUIDataAsset& Get();

	/** Returns the localized texts about specified end game to display on UI.
	 * @see UBmrUIDataAsset::EndGameTexts. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FText& GetEndGameText(EBmrEndGameState EndGameState) const;

	/** Returns the default avatar for the specified player type.
	 * @see UBmrUIDataAsset::DefaultAvatars. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UTexture2D* GetDefaultAvatar(EBmrPlayerType PlayerType) const;

	/** Returns the icon for the specified powerup type to display in the UI.
	 * @see UBmrUIDataAsset::PowerupIcons. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UTexture2D* GetPowerupIcon(FBmrPowerupTag PowerupTag) const;

protected:
	/** Contains the localized texts about specified end game to display on UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	TMap<EBmrEndGameState, FText> EndGameTexts;

	/** Contains all default avatar for the specified player type when playing against AI or without internet connection. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	TMap<EBmrPlayerType, TObjectPtr<class UTexture2D>> DefaultAvatars;

	/** Contains all icons for powerup types to display in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	TMap<FBmrPowerupTag, TObjectPtr<class UTexture2D>> PowerupIcons;
};