// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "GameplayTagContainer.h"

#include "NmmStateTag.generated.h"

/**
 * The tag that represents specific New Main Menu state type.
 * Is local for each player in session and not replicated.
 */
USTRUCT(BlueprintType, meta = (Categories = "NMM.State"))
struct NEWMAINMENU_API FNmmStateTag : public FGameplayTag
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * State Tags
	 ********************************************************************************************* */

	static const FNmmStateTag None; // Empty tag, nothing chosen by default
	static const FGameplayTag ParentTag; // Parent tag for binding to tag changes and filtering
	static const FNmmStateTag BasicMenu; // Basic menu: gameplay camera, Play/Settings/Quit visible, no cinematics loaded
	static const FNmmStateTag Idle; // Cinematic lobby: spot camera, character selection buttons
	static const FNmmStateTag Transition; // Cinematic logbby: camera moving between spots on rail
	static const FNmmStateTag Cinematic; // Cinematic part playing after Play pressed

	/*********************************************************************************************
	 * Logic
	 ********************************************************************************************* */

	/** Default constructor. */
	FNmmStateTag() = default;

	/** Custom constructor to set all members values. */
	FNmmStateTag(const FGameplayTag& Tag);

	/** Combines two tags into a container for use with HasAny(), e.g.: TagContainer.HasAny(Tag1 | Tag2). */
	friend NEWMAINMENU_API FGameplayTagContainer operator|(const FNmmStateTag& A, const FNmmStateTag& B);

	/** Allows chaining multiple tags, e.g.: TagContainer.HasAny(Tag1 | Tag2 | Tag3). */
	friend NEWMAINMENU_API FGameplayTagContainer operator|(const FGameplayTagContainer& Container, const FNmmStateTag& Tag);
};
