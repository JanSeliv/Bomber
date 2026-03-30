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
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Widget_HUD);
	} // namespace UI

	namespace Event
	{
		/** Event that activates collecting powerup ability*/
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Powerup_Collected);

		/** Event that attempts to activate the bomb ability*/
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bomb_Placed);

		/** Event that fires on death*/
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Death);

		/** Event that fires when new player connected to server while match was already in progress, is called on GeneratedMap ASC */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_PostLogin);

		/** Event that fires when game over condition met (all humans dead or last player standing), is called on owner pawn ASC */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_OnEndGame);

		/** Event that fires when user pressed Menu button on HUD to return to Main Menu, is called on local player ASC */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HUD_MenuButtonPressed);

		/** Event that fires when user pressed Restart button on HUD to restart the match, is called on local player ASC */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HUD_RestartButtonPressed);

		/** Event that fires when the current game state was changed (Menu, GameStarting, InGame, EndGame), check via Payload.InstigatorTags.HasTag() */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameState_Changed);

		/** Event that fires when any pawn is spawned, possessed, and replicated, obtain pawn from Payload.Instigator */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_PawnReady);

		/** Event that fires only for locally-controlled player-controlled pawns when ready, obtain pawn from Payload.Instigator */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_LocalPawnReady);

		/** Event that fires when the game difficulty tag was changed, check via Payload.InstigatorTags.HasTag/MatchesTag */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Changed);

		/** Event that fires when Generated Map is initialized and its data assets are loaded, is also called in editor */
		BOMBER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GeneratedMap_Ready);
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