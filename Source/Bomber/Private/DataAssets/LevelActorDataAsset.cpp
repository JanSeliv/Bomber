﻿// Copyright (c) Yevhenii Selivanov.

#include "DataAssets/LevelActorDataAsset.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "GameFramework/Actor.h"
//---
#if WITH_EDITOR
#include "MyUnrealEdEngine.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(LevelActorDataAsset)

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

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		UMyUnrealEdEngine::GOnAnyDataAssetChanged.Broadcast();
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
	if (!UUtilsLibrary::IsEditorNotPieWorld())
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

// Return first found row by specified level types
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

// Returns first found row by given predicate function
const ULevelActorRow* ULevelActorDataAsset::GetRowByPredicate(const TFunctionRef<bool(const ULevelActorRow&)>& Predicate) const
{
	for (const ULevelActorRow* RowIt : RowsInternal)
	{
		if (RowIt
		    && RowIt->IsValid()
		    && Predicate(*RowIt))
		{
			return RowIt;
		}
	}
	return nullptr;
}
