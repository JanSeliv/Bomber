// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// Bomber
#include "Structures/BmrManageableWidgetData.h"
#include "Structures/BmrPowerupTag.h"

#include "BmrUIDataAsset.generated.h"

enum class EBmrEndGameState : uint8;
enum class EBmrPlayerType : uint8;

/**
 * Contains in-game UI data.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrUIDataAsset final : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the UI data asset. */
	static const UBmrUIDataAsset& Get();

	/** Returns widget data associated with the given tag, or invalid widget data if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FBmrManageableWidgetData& GetWidgetDataByTag(FGameplayTag InTag) const;

	/** Returns widget data associated with the given widget class, or invalid widget data if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FBmrManageableWidgetData& GetWidgetDataByClass(TSubclassOf<class UUserWidget> WidgetClass) const;

	/** Returns all widgets to create, each identified by a gameplay tag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE TArray<FBmrManageableWidgetData>& GetAllWidgetData() const { return AllWidgetData; }

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
	/** List of all widgets to create, each identified by a gameplay tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, TitleProperty = "WidgetTag"))
	TArray<FBmrManageableWidgetData> AllWidgetData;

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