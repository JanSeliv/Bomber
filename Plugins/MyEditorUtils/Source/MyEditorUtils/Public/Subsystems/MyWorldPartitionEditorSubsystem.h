// Copyright (c) Yevhenii Selivanov

#pragma once

#include "EditorSubsystem.h"

#include "MyWorldPartitionEditorSubsystem.generated.h"

/**
 * Handles first-time auto-loading of World Partition regions in editor.
 * When a WP level is opened with no saved regions, loads the entire editor world bounds
 * so the level is visible without manual region selection.
 * Loaded regions are persisted by engine to EditorPerProjectUserSettings.ini on editor close,
 * so subsequent opens restore them normally and this subsystem does nothing.
 */
UCLASS(Transient)
class MYEDITORUTILS_API UMyWorldPartitionEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Binds to world initialization delegate to auto-load WP regions */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Unbinds from world initialization delegate */
	virtual void Deinitialize() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Binds to per-world WP initialized event for editor worlds */
	void OnPostWorldInitialization(class UWorld* World, const struct FWorldInitializationValues IVS);

	/** Auto-loads all WP regions on first open when no saved regions exist */
	void OnWorldPartitionInitialized(class UWorldPartition* InWorldPartition);
};