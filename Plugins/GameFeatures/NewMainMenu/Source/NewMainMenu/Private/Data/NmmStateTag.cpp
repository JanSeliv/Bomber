// Copyright (c) Yevhenii Selivanov

#include "Data/NmmStateTag.h"

// UE
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NmmStateTag)

// The NMM State tag that contains nothing chosen by default
const FNmmStateTag FNmmStateTag::None = EmptyTag;

// The parent tag for all NMM state tags
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_NMMState, "NMM.State", "Parent tag for all New Main Menu state tags");
const FGameplayTag FNmmStateTag::ParentTag = TAG_NMMState.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_NMMState_BasicMenu, "NMM.State.BasicMenu", "Basic menu: gameplay camera, Play/Settings/Quit visible, no cinematics loaded");
const FNmmStateTag FNmmStateTag::BasicMenu = TAG_NMMState_BasicMenu.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_NMMState_Transition, "NMM.State.Transition", "Camera moving between spots on rail");
const FNmmStateTag FNmmStateTag::Transition = TAG_NMMState_Transition.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_NMMState_Idle, "NMM.State.Idle", "Cinematic lobby: spot camera, character selection buttons");
const FNmmStateTag FNmmStateTag::Idle = TAG_NMMState_Idle.GetTag();

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_NMMState_Cinematic, "NMM.State.Cinematic", "Cinematic part playing after Play pressed");
const FNmmStateTag FNmmStateTag::Cinematic = TAG_NMMState_Cinematic.GetTag();

// Custom constructor to set all members values
FNmmStateTag::FNmmStateTag(const FGameplayTag& Tag)
    : FGameplayTag(Tag)
{
}

// Combines two tags into a container for use with HasAny(), e.g.: TagContainer.HasAny(Tag1 | Tag2)
FGameplayTagContainer operator|(const FNmmStateTag& A, const FNmmStateTag& B)
{
	FGameplayTagContainer Container;
	Container.AddTag(A);
	Container.AddTag(B);
	return Container;
}

// Allows chaining multiple tags, e.g.: TagContainer.HasAny(Tag1 | Tag2 | Tag3)
FGameplayTagContainer operator|(const FGameplayTagContainer& Container, const FNmmStateTag& Tag)
{
	FGameplayTagContainer NewContainer = Container;
	NewContainer.AddTag(Tag);
	return NewContainer;
}
