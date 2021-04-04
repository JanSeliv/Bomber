// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameUserSettings.h"
//---
#include "Structures/SettingsRow.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "MyGameUserSettings.generated.h"

/**
 *
 */
UCLASS()
class BOMBER_API USettingsDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the settings data asset. */
	static const USettingsDataAsset& Get();

	/**
	 * @see USettingsDataAsset::SettingsDataTableInternal */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void GenerateSettingsArray(TMap<FName, FSettingsRow>& OutRows) const;

	/** Delegate to react on changing settings data table. */
	DECLARE_DYNAMIC_DELEGATE(FOnDataTableChanged);

	/** Get a multicast delegate that is called any time the data table changes.
	 * @warning DevelopmentOnly */
	UFUNCTION(BlueprintCallable, BlueprintPure = "false", Category = "C++", meta = (DevelopmentOnly))
	void BindOnDataTableChanged(const FOnDataTableChanged& EventToBind) const;

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UDataTable* GetSettingsDataTable() const { return SettingsDataTableInternal; }

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
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Returns the game user settings.
	 * Is init once and can not be destroyed. */
	static UMyGameUserSettings& Get();

	/**  */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetter, int32, Param);

	/**  */
	DECLARE_DYNAMIC_DELEGATE_RetVal(int32, FOnGetter);

	/**  */
	DECLARE_DYNAMIC_DELEGATE_RetVal(UObject*, FOnObjectContext);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	UObject* GetObjectContext(FName TagName) const;

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetOption(FName TagName, int32 InValue);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	int32 GetOption(FName TagName) const;

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Table Rows", ShowOnlyInnerProperties))
	TMap<FName, FSettingsRow> SettingsTableRowsInternal; //[D]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Loads the user settings from persistent storage */
	virtual void LoadSettings(bool bForceReload) override;

	/** Returns the amount of settings rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetSettingsTableRowsNum() const { return SettingsTableRowsInternal.Num(); }

	/** Returns the found num by specified tag.
	 * @param TagName The key by which the row will be find.
	 * @see UMyGameUserSettings::SettingsTableRowsInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FSettingsRow FindSettingsTableRow(FName TagName) const;

	/**
	 * Called whenever the data of a table has changed, this calls the OnDataTableChanged() delegate and per-row callbacks.
	 * @warning DevelopmentOnly */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly))
	void OnDataTableChanged();
};
