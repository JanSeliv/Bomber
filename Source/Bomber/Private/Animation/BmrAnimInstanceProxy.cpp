// Copyright (c) Yevhenii Selivanov

#include "Animation/BmrAnimInstanceProxy.h"

// GFPM
#include "GfpmUtils.h"

// UE
#include "Animation/AnimClassInterface.h"
#include "Animation/BlendSpace.h"
#include "UObject/UnrealType.h"

// Releases every blend space owned by given plugin, from the proxy sync group cache and the blend space player nodes
void FBmrAnimInstanceProxy::ResetBlendSpacesInModule(const UGameFeatureData* GameFeatureData)
{
	if (!ensureMsgf(GameFeatureData, TEXT("ASSERT: [%i] %hs:\n'GameFeatureData' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	UObject* AnimInstanceOwner = GetAnimInstanceObject();
	const IAnimClassInterface* AnimClass = AnimInstanceOwner ? IAnimClassInterface::GetFromClass(AnimInstanceOwner->GetClass()) : nullptr;
	if (!AnimClass)
	{
		// Native anim instance with no generated node properties to scan
		return;
	}

	bool bReleasedAny = false;
	for (const FStructProperty* AnimProperty : AnimClass->GetAnimNodeProperties())
	{
		void* NodePtr = AnimProperty->ContainerPtrToValuePtr<void>(AnimInstanceOwner);
		for (const FObjectProperty* ObjectProperty : TFieldRange<FObjectProperty>(AnimProperty->Struct))
		{
			if (!ObjectProperty
			    || !ObjectProperty->PropertyClass
			    || !ObjectProperty->PropertyClass->IsChildOf<UBlendSpace>())
			{
				// Not blendspace property
				continue;
			}

			// Reset current BlendSpace and the node's PreviousBlendSpace object properties
			const UObject* CachedBlendSpace = ObjectProperty->GetObjectPropertyValue_InContainer(NodePtr);
			if (CachedBlendSpace
			    && UGfpmUtils::IsInGameFeatureModule(CachedBlendSpace, GameFeatureData))
			{
				ObjectProperty->SetObjectPropertyValue_InContainer(NodePtr, nullptr);
				bReleasedAny = true;
			}
		}
	}

	if (bReleasedAny)
	{
		ResetSync();
	}
}
