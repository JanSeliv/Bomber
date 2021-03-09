// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameUserSettings.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
//---
#include "Bomber.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "MyGameUserSettings.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsFunction
{
	GENERATED_BODY()

	/**  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ShowOnlyInnerProperties))
	TSubclassOf<UObject> Class; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ShowOnlyInnerProperties))
	FName Name; //[D]
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSettingsRow : public FTableRowBase
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ShowOnlyInnerProperties))
	FGameplayTag Tag; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (ShowOnlyInnerProperties))
	FSettingsFunction FunctionPicker; //[D]
};

/**
 *
 */
UCLASS()
class BOMBER_API USettingsDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * @see USettingsDataAsset::SettingsDataTableInternal */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void GenerateSettingsArray(TArray<FSettingsRow>& OutRows) const;

	/** Delegate to react on changing settings data table. */
	DECLARE_DYNAMIC_DELEGATE(FOnDataTableChanged);

	/** Get a multicast delegate that is called any time the data table changes.
	 * @warning DevelopmentOnly */
	UFUNCTION(BlueprintCallable, BlueprintPure = "false", Category = "C++", meta = (DevelopmentOnly))
	void BindOnDataTableChanged(const FOnDataTableChanged& EventToBind) const;

	/**
	 * @see USettingsDataAsset::WidthByScreen */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetWidthByScreen() const { return WidthByScreenInternal; }

	/**
	 * @see USettingsDataAsset::HeightByScreen */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetHeightByScreen() const { return HeightByScreenInternal; }

	/**
	 * @see USettingsDataAsset::Title */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FText GetTitle() const { return TitleInternal; }

protected:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Data Table", ShowOnlyInnerProperties))
	class UDataTable* SettingsDataTableInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Width By Screen", ShowOnlyInnerProperties))
	float WidthByScreenInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Height By Screen", ShowOnlyInnerProperties))
	float HeightByScreenInternal; //[D]

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Title", ShowOnlyInnerProperties))
	FText TitleInternal; //[D]
};

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UMyGameUserSettings final : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/** Loads the user settings from persistent storage */
	virtual void LoadSettings(bool bForceReload) override;

#if WITH_EDITOR
	/**  */
	UFUNCTION()
	void TemplateSettingsSetter(const int32 ChosenID) { }
#endif

protected:
	/**
	 * Called whenever the data of a table has changed, this calls the OnDataTableChanged() delegate and per-row callbacks.
	 * @warning DevelopmentOnly */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly))
	void OnDataTableChanged();

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SomeFunction(int32 IntParam)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- IS CALLED ---"));
	}
};
