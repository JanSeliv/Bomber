// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/WidgetComponent.h"

#include "BmrPlayerNameWidgetComponent.generated.h"

enum class EBmrCurrentGameState : uint8;

/**
 * 3D widget component specialized for displaying player names above characters.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UBmrPlayerNameWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UBmrPlayerNameWidgetComponent();

public:
	/** Activates the widget component with further syncing it with the player state's name and PlayerId. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void Init(class ABmrPlayerState* PlayerState);

	/** Applies new widget visibility based on the current game state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void UpdateVisibility();

protected:
	/** Associated player state for this widget component.
	 * Is cached for syncing with the player name and PlayerId. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class ABmrPlayerState> AssociatedPlayerState = nullptr;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/**  Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Listen to manage the component visibility. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Is called when all game widgets are initialized to handle UI-related logic.
	 * Is not called on remote clients. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnWidgetsInitialized();

	/** Called when changed Character's name to update the widget. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPlayerNameChanged(FName NewNickname);

	/** Called when changed Character's PlayerId to update the widget. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPlayerIdChanged(int32 NewPlayerId);

	/** Called when changed character Dead status is changed to update the widget visibility. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPlayerDeadChanged(bool bIsDead);
};