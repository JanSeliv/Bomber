// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "NewAIUtils.generated.h"

/**
 * Static helper functions about New AI.
 */
UCLASS()
class NEWAI_API UNewAIUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Object getters
	 ********************************************************************************************* */
public:
	/** Returns New AI subsystem that provides access to the most important data like Data Asset. */
	UFUNCTION(BlueprintCallable, Category = "C++", DisplayName = "Get NewAI Base Subsystem", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static class UNewAIBaseSubsystem* GetBaseSubsystem(const UObject* OptionalWorldContext = nullptr);
};
