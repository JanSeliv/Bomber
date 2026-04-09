// Copyright (c) Yevhenii Selivanov.

#include "DataAssets/BmrLevelActorDataAsset.h"

// Bomber
#include "MyUtilsLibraries/UtilsLibrary.h"

#if WITH_EDITOR
#include "BmrUnrealEdEngine.h"
#endif

// UE
#include "GameFramework/Actor.h"
#include "UObject/AssetRegistryTagsContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrLevelActorDataAsset)

// Adds ActorClass and ActorType as asset registry tags for discovery without loading
void UBmrLevelActorDataAsset::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

	Context.AddTag(FAssetRegistryTag(ActorClassTag, GetPathNameSafe(ActorClass), FAssetRegistryTag::TT_Alphabetical));
	Context.AddTag(FAssetRegistryTag(ActorTypeTag, UEnum::GetValueAsString(ActorType), FAssetRegistryTag::TT_Alphabetical));
	Context.AddTag(FAssetRegistryTag(RowTypeTag, RowType ? RowType->GetPathName() : TEXT(""), FAssetRegistryTag::TT_Alphabetical));
}

#if WITH_EDITOR // [IsEditorNotPieWorld]
// Called to notify on any data asset changes
void UBmrBaseDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		UBmrUnrealEdEngine::GOnAnyDataAssetChanged.Broadcast();
	}
}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
