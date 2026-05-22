// Copyright (c) Yevhenii Selivanov

#pragma once

#include "EditorSubsystem.h"

#include "GfpmLoaderEditorSubsystem.generated.h"

/**
 * Snapshots editor world's active GFP set before PIE starts and restores it once PIE shuts down, so PIE-side console commands and tag changes that mutated global GFP state do not leak past the play session.
 * Editor-scoped so it excludes -game and builds.
 */
UCLASS(DisplayName = "Game Feature Plugins Loader Editor Subsystem")
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmLoaderEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/** When subsystem initializes. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** When subsystem is destroyed. */
	virtual void Deinitialize() override;

protected:
	/** PIE-begin subscription handle */
	FDelegateHandle BeginPIEHandle;

	/** PIE-end subscription handle */
	FDelegateHandle EndPIEHandle;

	/** Pre-PIE plugin state for restoring editor baseline after session */
	TSet<FName> ActivePluginsSnapshot;

	/** When PIE session is about to start */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	void OnPreBeginPIE(bool bIsSimulating);

	/** When PIE session has ended */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	void OnEndPIE(bool bIsSimulating);
};
