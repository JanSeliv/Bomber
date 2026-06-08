// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Animation/AnimInstanceProxy.h"

class UGameFeatureData;

/**
 * Animation proxy for Bomber characters.
 */
struct BOMBER_API FBmrAnimInstanceProxy : public FAnimInstanceProxy
{
	FBmrAnimInstanceProxy() = default;
	explicit FBmrAnimInstanceProxy(UAnimInstance* InAnimInstance)
	    : FAnimInstanceProxy(InAnimInstance) { }

	/** Releases every blend space owned by given plugin, from the proxy sync group cache and the blend space player nodes. */
	void ResetBlendSpacesInModule(const UGameFeatureData* GameFeatureData);
};
