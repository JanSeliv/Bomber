// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"

// Bomber
#include "Structures/BmrPowerupTag.h"
#include "Structures/ManageableWidgetData.h"

#include "UIDataAsset.generated.h"

enum class EEndGameState : uint8;
enum class EPlayerType : uint8;

/**
 * Contains in-game UI data.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UUIDataAsset final : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the UI data asset. */
	static const UUIDataAsset& Get();

	/** Returns widget data associated with the given tag, or invalid widget data if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FManageableWidgetData& GetWidgetDataByTag(FGameplayTag InTag) const;

	/** Returns widget data associated with the given widget class, or invalid widget data if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FManageableWidgetData& GetWidgetDataByClass(TSubclassOf<class UUserWidget> WidgetClass) const;

	/** Returns all widgets to create, each identified by a gameplay tag.
	 * @see UUIDataAsset::AllWidgetData. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE TArray<FManageableWidgetData>& GetAllWidgetData() const { return AllWidgetData; }

	/** Returns the localized texts about specified end game to display on UI.
	 * @see UUIDataAsset::EndGameTexts. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FText& GetEndGameText(EEndGameState EndGameState) const;

	/** Returns the default avatar for the specified player type.
	 * @see UUIDataAsset::DefaultAvatarsInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UTexture2D* GetDefaultAvatar(EPlayerType PlayerType) const;

	/** Returns the icon for the specified powerup type to display in the UI.
	 * @see UUIDataAsset::PowerupIconsInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UTexture2D* GetPowerupIcon(FBmrPowerupTag PowerupTag) const;

protected:
	/** List of all widgets to create, each identified by a gameplay tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets", meta = (BlueprintProtected))
	TArray<FManageableWidgetData> AllWidgetData;

	/** Contains the localized texts about specified end game to display on UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "End-Game Names"))
	TMap<EEndGameState, FText> EndGameTextsInternal;

	/** Contains all default avatar for the specified player type when playing against AI or without internet connection. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Default Avatars"))
	TMap<EPlayerType, TObjectPtr<class UTexture2D>> DefaultAvatarsInternal;

	/** Contains all icons for powerup types to display in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Powerup Icons"))
	TMap<FBmrPowerupTag, TObjectPtr<class UTexture2D>> PowerupIconsInternal;
};