// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CurveTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SaveGame.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayUtilsLibrary)

/*********************************************************************************************
 * Multiplayer Helpers
 ********************************************************************************************* */

// Returns true if this instance has authority
bool UGameplayUtilsLibrary::IsServer()
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	return World && !World->IsNetMode(NM_Client);
}

// Returns amount of players (host + clients) playing this game
int32 UGameplayUtilsLibrary::GetPlayersInMultiplayerNum()
{
	int32 PlayersNum = 0;

	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return PlayersNum;
	}

	for (const APlayerState* PlayerStateIt : GameState->PlayerArray)
	{
		if (PlayerStateIt && !PlayerStateIt->IsABot())
		{
			++PlayersNum;
		}
	}

	return PlayersNum;
}

/*********************************************************************************************
 * Actor Helpers
 ********************************************************************************************* */

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
AActor* UGameplayUtilsLibrary::GetAttachedActorByClass(const AActor* ParentActor, TSubclassOf<AActor> ChildActorClass, bool bIncludeDescendants/* = false*/)
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

// Is useful for animating actor's transform from values stored in the Curve Table
bool UGameplayUtilsLibrary::ApplyTransformFromCurveTable(AActor* InActor, const FTransform& CenterWorldTransform, UCurveTable* CurveTable, float TotalSecondsSinceStart)
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
		|| !ensureMsgf(InActor, TEXT("ASSERT: [%i] %hs:\n'InActor' is not valid!"), __LINE__, __FUNCTION__)
		|| !ensureMsgf(TotalSecondsSinceStart >= 0.f, TEXT("ASSERT: [%i] %hs:\n'TotalSecondsSinceStart' must be greater or equal to 0!"), __LINE__, __FUNCTION__)
		|| !ensureMsgf(CenterWorldTransform.IsValid(), TEXT("ASSERT: [%i] %hs:\n'CenterWorldTransform' is not valid, it should be initial transform of actor to apply animation on it!"), __LINE__, __FUNCTION__))
	{
		return false;
	}

	static const TArray<FName> LocationRows = {FName("LocationX"), FName("LocationY"), FName("LocationZ")};
	static const TArray<FName> RotationRows = {FName("RotationPitch"), FName("RotationYaw"), FName("RotationRoll")};
	static const TArray<FName> ScaleRows = {FName("ScaleX"), FName("ScaleY"), FName("ScaleZ")};

	FVector NewLocation = FVector::ZeroVector;
	FVector NewScale = FVector::OneVector;
	FVector RotationValues = FVector::ZeroVector;

	float EvaluatedValue = 0.f;

	auto EvaluateCurveRow = [](UCurveTable* CurveTable, FName RowName, float InXY, float& OutValue) -> bool
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
		if (!EvaluateCurveRow(CurveTable, LocationRows[Index], TotalSecondsSinceStart, EvaluatedValue))
		{
			return false;
		}
		NewLocation[Index] = EvaluatedValue;
	}

	for (int32 Index = 0; Index < RotationRows.Num(); ++Index)
	{
		if (!EvaluateCurveRow(CurveTable, RotationRows[Index], TotalSecondsSinceStart, EvaluatedValue))
		{
			return false;
		}
		RotationValues[Index] = EvaluatedValue;
	}
	const FRotator NewRotation = FRotator::MakeFromEuler(RotationValues);

	for (int32 Index = 0; Index < ScaleRows.Num(); ++Index)
	{
		if (!EvaluateCurveRow(CurveTable, ScaleRows[Index], TotalSecondsSinceStart, EvaluatedValue))
		{
			return false;
		}
		NewScale[Index] = EvaluatedValue;
	}

	FTransform WorldTransform = FTransform::Identity;
	WorldTransform.SetLocation(CenterWorldTransform.GetLocation() + NewLocation);
	WorldTransform.SetRotation(CenterWorldTransform.GetRotation() * NewRotation.Quaternion());
	WorldTransform.SetScale3D(CenterWorldTransform.GetScale3D() * NewScale);

	InActor->SetActorTransform(WorldTransform);

	return true;
}

/*********************************************************************************************
 * Save Helpers
 ********************************************************************************************* */

// Completely removes given save data and creates new empty one
USaveGame* UGameplayUtilsLibrary::ResetSaveGameData(class USaveGame* SaveGame, const FString& SaveSlotName, int32 SaveSlotIndex)
{
	if (!ensureMsgf(SaveGame, TEXT("ASSERT: [%i] %hs:\n'SaveGame' is not valid!"), __LINE__, __FUNCTION__))
	{
		return nullptr;
	}

	// Remove the data from the disk
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveSlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveSlotIndex);
	}

	// Kill current save game object
	SaveGame->ConditionalBeginDestroy();

	// Create new save game object
	SaveGame = UGameplayStatics::CreateSaveGameObject(SaveGame->GetClass());
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, SaveSlotName, SaveSlotIndex);

	return SaveGame;
}

// Returns true if given object's config properties can be saved to the config file
bool UGameplayUtilsLibrary::CanSaveConfig(const UObject* Object)
{
	/*********************************************************************************************
	 * UObject::SaveConfig() checks
	 ********************************************************************************************* */

	const UClass* ObjectClass = Object ? Object->GetClass() : nullptr;
	if (!ObjectClass
		|| !ObjectClass->HasAnyClassFlags(CLASS_Config))
	{
		// Object is not marked with Config flag
		return false;
	}

	const FString ConfigName = ObjectClass->GetConfigName();
	if (ConfigName.IsEmpty())
	{
		// Config is not set
		return false;
	}

	const FString DestIniFilename = Object->GetDefaultConfigFilename();
	if (!FPaths::FileExists(DestIniFilename)
		|| IFileManager::Get().IsReadOnly(*DestIniFilename))
	{
		// Config was not found or is read-only
		return false;
	}

	/*********************************************************************************************
	 * FConfigContext::PerformLoad() checks
	 ********************************************************************************************* */

	// Attempt to load the configuration file
	FConfigFile ConfigFile;
	ConfigFile.Read(*DestIniFilename);
	if (ConfigFile.IsEmpty())
	{
		// Config is not found, meaning DistIniFilename path is not valid
		return false;
	}

	static const FString SectionsToSaveString = TEXT("SectionsToSave");
	const FConfigSection* SectionsToSaveSection = ConfigFile.FindSection(SectionsToSaveString);
	bool bLocalSaveAllSections = false;
	if (SectionsToSaveSection)
	{
		static const FString SaveAllSectionsKey = TEXT("bCanSaveAllSections");
		const FConfigValue* SaveAllSectionsValue = SectionsToSaveSection->Find(*SaveAllSectionsKey);
		if (SaveAllSectionsValue)
		{
			bLocalSaveAllSections = FCString::ToBool(*SaveAllSectionsValue->GetValue());
		}
	}

	static const FString UserName = TEXT("User");
	const bool bIsUserFile = ConfigName.Contains(UserName);

	static const FString EditorName = TEXT("Editor");
	const bool bIsEditorSettingsFile = ConfigName.Contains(EditorName) && !ConfigName.EndsWith(EditorName + TEXT(".ini"));

	// Return true if it's allowed to save all sections or if it's a user/editor-specific config file
	return bLocalSaveAllSections || bIsUserFile || bIsEditorSettingsFile;
}

// Saves the given object's config properties to the config file
void UGameplayUtilsLibrary::SaveConfig(UObject* Object)
{
	const bool bCanSaveConfig = CanSaveConfig(Object);
	if (!ensureMsgf(bCanSaveConfig, TEXT("ASSERT: [%i] %hs:\n'Object' can not be saved to the config file!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	Object->SaveConfig();
}
