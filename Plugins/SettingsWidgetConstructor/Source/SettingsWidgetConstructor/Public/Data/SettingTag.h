// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameplayTagContainer.h"
//---
#include "SettingTag.generated.h"

/**
 * Used to require all such tags start with 'Settings.X' as it specified in USTRUCT meta.
 */
USTRUCT(BlueprintType, meta = (Categories = "Settings"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingTag : public FGameplayTag
{
	GENERATED_BODY()

	/** Default constructor. */
	FSettingTag() = default;

	/** Custom constructor to set all members values. */
	FSettingTag(const FGameplayTag& Tag)
		: FGameplayTag(Tag) {}

	/** Setting gag that contains nothing by default. */
	static const FSettingTag EmptySettingTag;
};

/**
 * Allows automatically add native setting tags at startup.
 */
struct SETTINGSWIDGETCONSTRUCTOR_API FGlobalSettingTags : public FGameplayTagNativeAdder
{
	FSettingTag ButtonSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag CheckboxSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag ComboboxSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag ScrollboxSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag SliderSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag TextLineSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag UserInputSettingTag = FSettingTag::EmptySettingTag;
	FSettingTag CustomWidgetSettingTag = FSettingTag::EmptySettingTag;

	/** Returns global setting tags as const ref.
	 * @see FGlobalSettingTags::GSettingTags. */
	static const FORCEINLINE FGlobalSettingTags& Get() { return GSettingTags; }

	/** Automatically adds native setting tags at startup. */
	virtual void AddTags() override;

	/** Virtual destructor. */
	virtual ~FGlobalSettingTags() = default;

private:
	/** The global of Setting tag categories. */
	static FGlobalSettingTags GSettingTags;
};
