// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
//---
#include "NewMainMenuHUDComponent.generated.h"

/**
 * Represents the HUD in the NewMain Menu module, where the Owner is HUD actor.
 * Is responsible for managing Main Menu widgets.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWMAINMENU_API UNewMainMenuHUDComponent : public UActorComponent
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Public functions
	 ********************************************************************************************* */
public:
	/** Default constructor. */
	UNewMainMenuHUDComponent();

	/** Returns HUD actor of this component. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class AMyHUD* GetHUD() const;
	class AMyHUD& GetHUDChecked() const;

	/** Returns the data asset that contains all the assets and tweaks of New Main Menu game feature.
	 * @see UNewMainMenuSubsystem::NewMainMenuDataAssetInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const class UNewMainMenuDataAsset* GetNewMainMenuDataAsset() const;

	/** Returns created Main Menu widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UNewMainMenuWidget* GetMainMenuWidget() const { return MainMenuWidgetInternal; }

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** Contains all the assets and tweaks of New Main Menu game feature. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "New Main Menu Data Asset"))
	TSoftObjectPtr<const class UNewMainMenuDataAsset> NewMainMenuDataAssetInternal = nullptr;

	/** Created Main Menu widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main Menu Widget"))
	TObjectPtr<class UNewMainMenuWidget> MainMenuWidgetInternal = nullptr;

	/** Created In Cinematic State widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "In Cinematic State Widget"))
	TObjectPtr<class UInCinematicStateWidget> InCinematicStateWidgetInternal = nullptr;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called. */
	virtual void OnRegister() override;
};
