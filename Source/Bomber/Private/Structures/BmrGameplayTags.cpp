// Copyright (c) Yevhenii Selivanov.

#include "Structures/BmrGameplayTags.h"

namespace BmrGameplayTags
{
	namespace UI
	{
		UE_DEFINE_GAMEPLAY_TAG(Widget_Settings, "UI.Widget.Settings");
		UE_DEFINE_GAMEPLAY_TAG(Widget_Nickname, "UI.Widget.Nickname");
		UE_DEFINE_GAMEPLAY_TAG(Widget_FpsCounter, "UI.Widget.FPSCounter");
		UE_DEFINE_GAMEPLAY_TAG(Widget_HUD, "UI.Widget.HUD");
	} // namespace UI

	namespace Event
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Powerup_Collected, "Event.Powerup.Collected", "Event that activates collecting powerup ability.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Bomb_Placed, "Event.Bomb.Placed", "Event that attempts to activate the bomb ability");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_Death, "Event.Player.Death", "Event that fires on death");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_PostLogin, "Event.Player.PostLogin", "Event that fires when new player connected to server while match was already in progress, is called on GeneratedMap ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_OnEndGame, "Event.Player.OnEndGame", "Event that fires when game over condition met (all humans dead or last player standing), is called on owner pawn ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(HUD_MenuButtonPressed, "Event.HUD.MenuButtonPressed", "Event that fires when user pressed Menu button on HUD to return to Main Menu, is called on local player ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(HUD_RestartButtonPressed, "Event.HUD.RestartButtonPressed", "Event that fires when user pressed Restart button on HUD to restart the match, is called on local player ASC");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameState_Changed, "Event.GameState.Changed", "Event that fires when the current game state was changed (Menu, GameStarting, InGame, EndGame), check via Payload.InstigatorTags.HasTag()");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_PawnReady, "Event.Player.PawnReady", "Event that fires when any pawn is spawned, possessed, and replicated, obtain pawn from Payload.Instigator");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_LocalPawnReady, "Event.Player.LocalPawnReady", "Event that fires only for locally-controlled player-controlled pawns when ready, obtain pawn from Payload.Instigator");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Changed, "Event.Difficulty.Changed", "Event that fires when the game difficulty tag was changed, check via Payload.InstigatorTags.HasTag/MatchesTag");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(GeneratedMap_Ready, "Event.GeneratedMap.Ready", "Event that fires when Generated Map is initialized and its data assets are loaded, is also called in editor");
	} // namespace Event

	namespace GameplayEffect
	{
		namespace Block
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingDamage, "GameplayEffect.Block.IncomingDamage", "Is added by UBmrPlayerDataAsset::BlockIncomingDamageEffectInternal, defines a temporary immunity effect for incoming damage, e.g., god mode, join game in progress etc");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement, "GameplayEffect.Block.Movement", "Is added by UBmrPlayerDataAsset::BlockMovementEffectInternal, defines that player movement is disabled, e.g., in menu, during 3-2-1 timer, when died etc.");
		} // namespace Block
	} // namespace GameplayEffect

	namespace SetByCaller
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Bomb_Damage, "SetByCaller.Bomb.Damage", "SetByCaller tag to set bomb damage amount, is set in Explosion gameplay effect and captured in damage execution");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Bomb_Duration, "SetByCaller.Bomb.Duration", "SetByCaller tag to set bomb duration in seconds, modifies original duration to compensate for network latency on server side");
	} // namespace SetByCaller

	namespace GameplayCue
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Bomb_Explosion, "GameplayCue.Bomb.Explosion", "Immediate visual feedback executed locally on all clients when bomb detonates to spawn VFXs and SFXs despite damage is server-authority only");
	} // namespace GameplayCue
} // namespace BmrGameplayTags