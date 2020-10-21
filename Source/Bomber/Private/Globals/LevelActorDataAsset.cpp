// Copyright 2020 Yevhenii Selivanov.

#include "Globals/LevelActorDataAsset.h"
//---
#include "Globals/SingletonLibrary.h"

#if WITH_EDITOR // [IsEditorNotPieWorld]
// Called to notify on any data asset changes
void UBomberDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		USingletonLibrary::GOnAnyDataAssetChanged.Broadcast();
	}
}
#endif //WITH_EDITOR [IsEditorNotPieWorld]

void ULevelActorDataAsset::GetRowsByLevelType(TArray<ULevelActorRow*>& OutRows, int32 LevelsTypesBitmask) const
{
	for (ULevelActorRow* const& RowIt : RowsInternal)
	{
		if (RowIt
			&& RowIt->Mesh //is not empty
			&& EnumHasAnyFlags(RowIt->LevelType, TO_ENUM(ELevelType, LevelsTypesBitmask)))
		{
			OutRows.Emplace(RowIt);
		}
	}
}
