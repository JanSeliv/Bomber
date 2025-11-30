// Copyright (c) Yevhenii Selivanov.

#pragma once

// UE
#include "NativeGameplayTags.h" // UE_DECLARE_GAMEPLAY_TAG_EXTERN

namespace BmrGameplayTags
{
	namespace UI
	{
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Widget_Settings);
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Widget_Nickname);
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Widget_FpsCounter);
	} // namespace UI

	namespace Event
	{
		/** Event that activates collecting powerup ability*/
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Powerup_Collected);

		/** Event that attempts to activate the bomb ability*/
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bomb_Placed);

		/** Event that fires on death*/
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Death);
	} // namespace Event

	namespace GameplayEffect
	{
		namespace Block
		{
			/** Is added by UBmrPlayerDataAsset::BlockIncomingDamageEffect, defines a temporary immunity effect for incoming damage, e.g., god mode, join game in progress etc. */
			BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(IncomingDamage);

			/** Is added by UBmrPlayerDataAsset::BlockMovementEffect, defines that player movement is disabled, e.g., in menu, during 3-2-1 timer, when died etc. */
			BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement);
		} // namespace Block
	} // namespace GameplayEffect

	namespace SetByCaller
	{
		/** SetByCaller tag to set bomb damage amount, is set in Explosion gameplay effect and captured in damage execution. */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bomb_Damage);

		/** SetByCaller tag to set bomb duration in seconds, modifies original duration to compensate for network latency on server side. */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bomb_Duration);
	} // namespace SetByCaller

	namespace GameplayCue
	{
		/** Immediate visual feedback executed locally on all clients when bomb detonates to spawn VFXs and SFXs despite damage is server-authority only. */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bomb_Explosion);
	} // namespace GameplayCue
} // namespace BmrGameplayTags