// Copyright 2021 Yevhenii Selivanov.

#include "Globals/LevelActorDataAsset.h"
//---
#include "Globals/SingletonLibrary.h"

#if WITH_EDITOR // [IsEditorNotPieWorld]
// Called to handle row changes
void ULevelActorRow::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

// Called to notify on any data asset changes
void UBomberDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		USingletonLibrary::GOnAnyDataAssetChanged.Broadcast();
	}
}

// Handle adding new rows of level actor data assets
void ULevelActorDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!ensureMsgf(RowClassInternal, TEXT("ASSERT: 'RowClassInternal' is not valid")))
	{
		return;
	}

	// Continue only if [IsEditorNotPieWorld]
	if (!USingletonLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	// Continue only if was added new row
	static const FString PropertyName = GET_MEMBER_NAME_STRING_CHECKED(ThisClass, RowsInternal);
	const FProperty* Property = PropertyChangedEvent.Property;
	if (!Property
	    || !Property->IsA<FArrayProperty>()
	    || PropertyChangedEvent.ChangeType != EPropertyChangeType::ArrayAdd
	    || Property->GetName() != PropertyName)
	{
		return;
	}

	// Initialize new row
	const int32 AddedAtIndex = PropertyChangedEvent.GetArrayIndex(PropertyName);
	if (RowsInternal.IsValidIndex(AddedAtIndex))
	{
		TObjectPtr<ULevelActorRow>& Row = RowsInternal[AddedAtIndex];
		if (!Row)
		{
			Row = NewObject<ULevelActorRow>(this, RowClassInternal, NAME_None, RF_Public | RF_Transactional);
		}
	}
}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

// Return rows by specified level types in the bitmask
void ULevelActorDataAsset::GetRowsByLevelType(TArray<ULevelActorRow*>& OutRows, int32 LevelsTypesBitmask) const
{
	for (ULevelActorRow* RowIt : RowsInternal)
	{
		if (RowIt
		    && RowIt->Mesh //is not empty
		    && EnumHasAnyFlags(RowIt->LevelType, TO_ENUM(ELevelType, LevelsTypesBitmask)))
		{
			OutRows.Emplace(RowIt);
		}
	}
}

// Return rows by specified level types in the bitmask
const ULevelActorRow* ULevelActorDataAsset::GetRowByLevelType(ELevelType LevelType) const
{
	TArray<ULevelActorRow*> Rows;
	GetRowsByLevelType(Rows, TO_FLAG(LevelType));
	return Rows.IsValidIndex(0) ? Rows[0] : nullptr;
}
