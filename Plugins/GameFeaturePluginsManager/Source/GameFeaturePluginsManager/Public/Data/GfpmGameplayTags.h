// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "NativeGameplayTags.h" // UE_DECLARE_GAMEPLAY_TAG_EXTERN

namespace GfpmGameplayTags
{
	namespace Event
	{
		/** Event that fires when the world Ability System Component becomes available. */
		GAMEFEATUREPLUGINSMANAGER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldASC_Ready);
	} // namespace Event
} // namespace GfpmGameplayTags