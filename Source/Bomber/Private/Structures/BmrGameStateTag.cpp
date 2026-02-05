// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrGameStateTag.h"

// UE
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameStateTag)

// The Game State tag that contains nothing chosen by default
const FBmrGameStateTag FBmrGameStateTag::None = EmptyTag;

// The parent tag for all game state tags
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameState, "GameState", "Parent tag for all game state tags");
const FGameplayTag FBmrGameStateTag::ParentTag = TAG_GameState.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameState_Menu, "GameState.Menu", "Is active while players are in Main-Menu");
const FBmrGameStateTag FBmrGameStateTag::Menu = TAG_GameState_Menu.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameState_GameStarting, "GameState.GameStarting", "Is active while players see count-down time (3-2-1)");
const FBmrGameStateTag FBmrGameStateTag::GameStarting = TAG_GameState_GameStarting.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameState_InGame, "GameState.InGame", "Is active during the active match");
const FBmrGameStateTag FBmrGameStateTag::InGame = TAG_GameState_InGame.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameState_EndGame, "GameState.EndGame", "Is active when the match is finished and players see their results of the game");
const FBmrGameStateTag FBmrGameStateTag::EndGame = TAG_GameState_EndGame.GetTag();

// Custom constructor to set all members values
FBmrGameStateTag::FBmrGameStateTag(const FGameplayTag& Tag)
    : FGameplayTag(Tag)
{
}

// Combines two tags into a container for use with HasAny(), e.g.: TagContainer.HasAny(Tag1 | Tag2)
FGameplayTagContainer operator|(const FBmrGameStateTag& A, const FBmrGameStateTag& B)
{
	FGameplayTagContainer Container;
	Container.AddTag(A);
	Container.AddTag(B);
	return Container;
}

// Allows chaining multiple tags, e.g.: TagContainer.HasAny(Tag1 | Tag2 | Tag3)
FGameplayTagContainer operator|(const FGameplayTagContainer& Container, const FBmrGameStateTag& Tag)
{
	FGameplayTagContainer NewContainer = Container;
	NewContainer.AddTag(Tag);
	return NewContainer;
}