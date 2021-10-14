// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "InputControlsWidget.generated.h"

/**
 * Allows player to rebind input mappings.
 */
UCLASS()
class BOMBER_API UInputControlsWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Display the Input Controls widget on UI. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OpenWidget();

	/** Close the Input Controls widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void CloseWidget();

protected:
	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/* Updates appearance dynamically in the editor. */
	virtual void SynchronizeProperties() override;

	/** Is called when visibility is changed for this widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnVisibilityChange(ESlateVisibility InVisibility);
};
