// Copyright (c) Yevhenii Selivanov

#include "NewAIUtils.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewAIUtils)

/*********************************************************************************************
 * Object getters
 ********************************************************************************************* */

// Returns New AI subsystem that provides access to the most important data like Data Asset
UNewAIBaseSubsystem* UNewAIUtils::GetBaseSubsystem(const UObject* OptionalWorldContext)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld(OptionalWorldContext);
	return World ? World->GetSubsystem<UNewAIBaseSubsystem>() : nullptr;
}
