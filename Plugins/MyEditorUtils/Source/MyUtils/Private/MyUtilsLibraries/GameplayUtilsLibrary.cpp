// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/GameplayUtilsLibrary.h"

// MyUtils
#include "MyUtilsLibraries/UtilsLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CurveTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayUtilsLibrary)

/*********************************************************************************************
 * Actor Helpers
 ********************************************************************************************* */

// Abstract getter that allows to obtain the static or skeletal mesh from given mesh component (base class of both)
class UStreamableRenderAsset* UGameplayUtilsLibrary::GetMesh(const UMeshComponent* MeshComponent)
{
	if (const USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponent))
	{
		return SkeletalMeshComponent->GetSkeletalMeshAsset();
	}

	if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
	{
		return StaticMeshComponent->GetStaticMesh();
	}

	return nullptr;
}

// Abstract method that allows set both static and skeletal meshes to the specified mesh component
void UGameplayUtilsLibrary::SetMesh(UMeshComponent* MeshComponent, UStreamableRenderAsset* MeshAsset)
{
	if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponent))
	{
		SkeletalMeshComponent->SetSkeletalMesh(Cast<USkeletalMesh>(MeshAsset));
	}
	else if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
	{
		StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(MeshAsset));
	}
}

// Returns the first child actor of the specified class
AActor* UGameplayUtilsLibrary::GetAttachedActorByClass(const AActor* ParentActor, TSubclassOf<AActor> ChildActorClass, bool bIncludeDescendants /* = false*/)
{
	if (!ensureMsgf(ParentActor, TEXT("ASSERT: [%i] %s:\n'!ParentActor' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	TArray<AActor*> AttachedActors;
	ParentActor->GetAttachedActors(AttachedActors);
	if (AttachedActors.IsEmpty())
	{
		return nullptr;
	}

	for (AActor* It : AttachedActors)
	{
		if (It && It->IsA(ChildActorClass))
		{
			return It;
		}

		if (bIncludeDescendants)
		{
			if (AActor* FoundActor = GetAttachedActorByClass(It, ChildActorClass, bIncludeDescendants))
			{
				return FoundActor;
			}
		}
	}

	return nullptr;
}

// Extracts transform values from a Curve Table at a specific time
bool UGameplayUtilsLibrary::GetTransformFromCurveTable(FTransform& OutTransform, UCurveTable* CurveTable, float TotalSecondsSinceStart)
{
	/* Example data (can be imported as csv into your Curve Table):

	    Name,0,0.1,0.2,0.5
	    LocationX,0,0.0,0.0,0
	    LocationY,0,0.0,0.0,0
	    LocationZ,0,0.0,0.0,0
	    RotationPitch,0,0.0,0.0,0
	    RotationYaw,0,0.0,0.0,0
	    RotationRoll,0,0.0,0.0,0
	    ScaleX,1,0.9,0.7,0
	    ScaleY,1,0.9,0.7,0
	    ScaleZ,1,0.9,0.7,0

	Where ScaleZ will change from `1` (at 0 sec) to `0` (at 0.5 sec) */

	if (!ensureMsgf(CurveTable, TEXT("ASSERT: [%i] %hs:\n'CurveTable' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(TotalSecondsSinceStart >= 0.f, TEXT("ASSERT: [%i] %hs:\n'TotalSecondsSinceStart' must be greater or equal to 0!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	static const TArray<FName> LocationRows = {TEXT("LocationX"), TEXT("LocationY"), TEXT("LocationZ")};
	static const TArray<FName> RotationRows = {TEXT("RotationPitch"), TEXT("RotationYaw"), TEXT("RotationRoll")};
	static const TArray<FName> ScaleRows = {TEXT("ScaleX"), TEXT("ScaleY"), TEXT("ScaleZ")};

	FVector NewLocation = FVector::ZeroVector;
	FVector NewScale = FVector::OneVector;
	FVector RotationValues = FVector::ZeroVector;

	float EvaluatedValue = 0.f;

	auto EvaluateCurveRow = [CurveTable](FName RowName, float InXY, float& OutValue) -> bool
	{
		FCurveTableRowHandle Handle;
		Handle.CurveTable = CurveTable;
		Handle.RowName = RowName;

		const FString ContextString = RowName.ToString();
		const FRealCurve* Curve = CurveTable ? Handle.CurveTable->FindCurve(RowName, ContextString) : nullptr;
		if (!Curve)
		{
			return false;
		}

		float MinTime = 0.f;
		float MaxTime = 0.f;
		Curve->GetTimeRange(MinTime, MaxTime);
		if (InXY >= MaxTime)
		{
			// The curve is finished
			return false;
		}

		return Handle.Eval(InXY, &OutValue, ContextString);
	};

	for (int32 Index = 0; Index < LocationRows.Num(); ++Index)
	{
		if (!EvaluateCurveRow(LocationRows[Index], TotalSecondsSinceStart, EvaluatedValue))
		{
			return false;
		}
		NewLocation[Index] = EvaluatedValue;
	}

	for (int32 Index = 0; Index < RotationRows.Num(); ++Index)
	{
		if (!EvaluateCurveRow(RotationRows[Index], TotalSecondsSinceStart, EvaluatedValue))
		{
			return false;
		}
		RotationValues[Index] = EvaluatedValue;
	}
	const FRotator NewRotation = FRotator::MakeFromEuler(RotationValues);

	for (int32 Index = 0; Index < ScaleRows.Num(); ++Index)
	{
		if (!EvaluateCurveRow(ScaleRows[Index], TotalSecondsSinceStart, EvaluatedValue))
		{
			return false;
		}
		NewScale[Index] = EvaluatedValue;
	}

	OutTransform = FTransform(NewRotation, NewLocation, NewScale);

	return true;
}

// Applies transform from Curve Table to actor around a center point
bool UGameplayUtilsLibrary::ApplyTransformFromCurveTable(AActor* InActor, const FTransform& CenterWorldTransform, UCurveTable* CurveTable, float TotalSecondsSinceStart)
{
	if (!ensureMsgf(InActor, TEXT("ASSERT: [%i] %hs:\n'InActor' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(CenterWorldTransform.IsValid(), TEXT("ASSERT: [%i] %hs:\n'CenterWorldTransform' is not valid, it should be initial transform of actor to apply animation on it!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	FTransform CurveTransform = FTransform::Identity;
	if (!GetTransformFromCurveTable(/*out*/ CurveTransform, CurveTable, TotalSecondsSinceStart))
	{
		return false;
	}

	// Combines curve animation with center transform to maintain stable pivot point
	FTransform WorldTransform = FTransform::Identity;
	WorldTransform.SetLocation(CenterWorldTransform.GetLocation() + CurveTransform.GetLocation());
	WorldTransform.SetRotation(CenterWorldTransform.GetRotation() * CurveTransform.GetRotation());
	WorldTransform.SetScale3D(CenterWorldTransform.GetScale3D() * CurveTransform.GetScale3D());
	InActor->SetActorTransform(WorldTransform);

	return true;
}

// Applies transform from Curve Table to scene component using actor transform as center
bool UGameplayUtilsLibrary::ApplyComponentTransformFromCurveTable(USceneComponent* InComponent, UCurveTable* CurveTable, float TotalSecondsSinceStart)
{
	const AActor* OwnerActor = InComponent ? InComponent->GetOwner() : nullptr;
	if (!ensureMsgf(OwnerActor, TEXT("ASSERT: [%i] %hs:\n'OwnerActor' is not valid!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	FTransform CurveTransform = FTransform::Identity;
	if (!GetTransformFromCurveTable(/*out*/ CurveTransform, CurveTable, TotalSecondsSinceStart))
	{
		return false;
	}

	// Combines curve animation with actor transform then converts to relative space for component
	const FTransform ActorWorldTransform = OwnerActor->GetActorTransform();
	FTransform WorldTransform = FTransform::Identity;
	WorldTransform.SetLocation(ActorWorldTransform.GetLocation() + CurveTransform.GetLocation());
	WorldTransform.SetRotation(ActorWorldTransform.GetRotation() * CurveTransform.GetRotation());
	WorldTransform.SetScale3D(ActorWorldTransform.GetScale3D() * CurveTransform.GetScale3D());
	const FTransform RelativeTransform = WorldTransform.GetRelativeTransform(ActorWorldTransform);
	InComponent->SetRelativeTransform(RelativeTransform);

	return true;
}

/*********************************************************************************************
 * Level Helpers
 ********************************************************************************************* */

// Returns true if the specified level is opened in the current world
bool UGameplayUtilsLibrary::IsLevelOpened(const TSoftObjectPtr<UWorld>& Level)
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	const FString CurrentLevelName = GetNameSafe(World);
	const FString LevelName = Level.GetAssetName();
	return CurrentLevelName.Contains(LevelName);
}

// Opens the specified level as listen server: Level?listen
void UGameplayUtilsLibrary::OpenListenServerLevel(const TSoftObjectPtr<UWorld>& Level, bool bForceLoad /* = false*/)
{
	if (!ensureMsgf(!Level.IsNull(), TEXT("ASSERT: [%i] %hs:\n'Level' is null!"), __LINE__, __FUNCTION__)
	    || (!bForceLoad && IsLevelOpened(Level)))
	{
		// Already in main level
		return;
	}

	constexpr bool bAbsolute = true;
	static const FString Options = TEXT("listen");
	UGameplayStatics::OpenLevelBySoftObjectPtr(UUtilsLibrary::GetPlayWorld(), Level, bAbsolute, Options);
}

/*********************************************************************************************
 * Ability System
 ********************************************************************************************* */

// Returns the tags owned by ASC that are descendants of Parent (Parent itself is excluded)
FGameplayTagContainer UGameplayUtilsLibrary::GetFilteredGameplayTags(const UAbilitySystemComponent* ASC, FGameplayTag Parent)
{
	if (!ASC
	    || !Parent.IsValid())
	{
		return FGameplayTagContainer::EmptyContainer;
	}

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);
	if (OwnedTags.IsEmpty())
	{
		return FGameplayTagContainer::EmptyContainer;
	}

	TStringBuilder<FName::StringBufferSize> ParentBuilder;
	Parent.GetTagName().ToString(ParentBuilder);
	const FStringView ParentView = ParentBuilder;
	const int32 ParentLen = ParentView.Len();

	FGameplayTagContainer Result = FGameplayTagContainer::EmptyContainer;
	for (const FGameplayTag& Tag : OwnedTags)
	{
		if (!Tag.IsValid())
		{
			continue;
		}

		TStringBuilder<FName::StringBufferSize> TagBuilder;
		Tag.GetTagName().ToString(TagBuilder);
		const FStringView TagView = TagBuilder;
		if (TagView.Len() > ParentLen
		    && TagView[ParentLen] == TEXT('.')
		    && TagView.StartsWith(ParentView, ESearchCase::CaseSensitive))
		{
			Result.AddTagFast(Tag);
		}
	}

	return Result;
}
