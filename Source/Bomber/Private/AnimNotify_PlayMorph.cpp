// Copyright 2021 Yevhenii Selivanov.

#include "AnimNotify_PlayMorph.h"

// Is called on an animation notify
void UAnimNotify_PlayMorph::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!ensureMsgf(MeshComp, TEXT("ASSERT: 'MeshComp' is not valid")))
	{
		return;
	}

	MeshComp->SetMorphTarget(MorphDataInternal.Morph, MorphDataInternal.StartValue);
}
