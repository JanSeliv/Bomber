// Copyright (c) Yevhenii Selivanov.

#include "DataAssets/BmrLevelActorDataAsset.h"

// Bomber
#include "MyUtilsLibraries/UtilsLibrary.h"

#if WITH_EDITOR
#include "BmrUnrealEdEngine.h"
#endif

// UE
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrLevelActorDataAsset)

#if WITH_EDITOR // [IsEditorNotPieWorld]
// Called to handle row changes
void UBmrLevelActorRow::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

// Called to notify on any data asset changes
void UBmrBaseDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		UBmrUnrealEdEngine::GOnAnyDataAssetChanged.Broadcast();
	}
}

// Handle adding new rows of level actor data assets
void UBmrLevelActorDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!ensureMsgf(RowClass, TEXT("ASSERT: 'RowClass' is not valid")))
	{
		return;
	}

	// Continue only if [IsEditorNotPieWorld]
	if (!UUtilsLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	// Continue only if was added new row
	static const FString PropertyName = GET_MEMBER_NAME_STRING_CHECKED(ThisClass, Rows);
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
	if (Rows.IsValidIndex(AddedAtIndex))
	{
		TObjectPtr<UBmrLevelActorRow>& Row = Rows[AddedAtIndex];
		if (!Row)
		{
			Row = NewObject<UBmrLevelActorRow>(this, RowClass, NAME_None, RF_Public | RF_Transactional);
		}
	}
}
#endif // WITH_EDITOR [IsEditorNotPieWorld]

// Return first found row by specified level types
void UBmrLevelActorDataAsset::GetRowsByLevelType(TArray<UBmrLevelActorRow*>& OutRows, int32 LevelsTypesBitmask) const
{
	for (UBmrLevelActorRow* RowIt : Rows)
	{
		if (RowIt
		    && RowIt->Mesh // is not empty
		    && EnumHasAnyFlags(RowIt->LevelType, TO_ENUM(EBmrLevelType, LevelsTypesBitmask)))
		{
			OutRows.Emplace(RowIt);
		}
	}
}

// Returns first found row by given predicate function
const UBmrLevelActorRow* UBmrLevelActorDataAsset::GetRowByPredicate(const TFunctionRef<bool(const UBmrLevelActorRow&)>& Predicate) const
{
	for (const UBmrLevelActorRow* RowIt : Rows)
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

// Return first found row by specified level types
const UBmrLevelActorRow* UBmrLevelActorDataAsset::GetRowByLevelType(EBmrLevelType LevelType) const
{
	return GetRowByPredicate([LevelType](const UBmrLevelActorRow& RowIt)
	{
		return RowIt.LevelType == LevelType || RowIt.LevelType == ELT::Max;
	});
}

// Return first found row by specified mesh
const UBmrLevelActorRow* UBmrLevelActorDataAsset::GetRowByMesh(const class UStreamableRenderAsset* Mesh) const
{
	return GetRowByPredicate([Mesh](const UBmrLevelActorRow& RowIt)
	{
		return RowIt.Mesh == Mesh;
	});
}
