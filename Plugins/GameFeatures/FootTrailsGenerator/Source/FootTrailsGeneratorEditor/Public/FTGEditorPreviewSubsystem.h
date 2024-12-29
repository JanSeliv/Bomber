// Copyright (c) Yevhenii Selivanov

#pragma once

#include "EditorSubsystem.h"
#include "FTGEditorPreviewSubsystem.generated.h"

/**
 * Handles foot trails automatic previewing in Editor before game starts.
 */
UCLASS()
class FOOTTRAILSGENERATOREDITOR_API UFTGEditorPreviewSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Runtime component to generate preview of foot trails. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "FootTrailGenerator"))
	TObjectPtr<class UFTGComponent> FootTrailGeneratorInternal = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Is called when the subsystem is initialized. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Is called when the subsystem is deinitialized. */
	virtual void Deinitialize() override;

	/** Is used to initialize the foot trails generator. */
	void OnBeginPlay(UWorld* World, struct FWorldInitializationValues WorldInitializationValues);

	/** Is used to destroy the foot trails generator. */
	void OnEndPlay(UWorld* World, bool bArg, bool bCond);

	/** Called when Generated Map is initialized and ready to be used, is also called in editor. */
	UFUNCTION()
	void OnGeneratedMapReady(class AGeneratedMap* GeneratedMap);
};
