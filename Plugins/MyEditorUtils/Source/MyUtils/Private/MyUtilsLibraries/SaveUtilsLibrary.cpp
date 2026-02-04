// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/SaveUtilsLibrary.h"

// My Utils
#include "MyUtilsLibraries/AsyncLoadUtilsLibrary.h"

// UE
#include "GameFramework/SaveGame.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveUtilsLibrary)

// Is code alternative of blueprintable UGameplayStatics::AsyncLoadGameFromSlot, which does the same, but ensures callback will be called in the correct world context, even in PIE multiplayer
void USaveUtilsLibrary::AsyncLoadGameFromSlot(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex, const FAsyncLoadGameFromSlot& Callback)
{
	if (!ensureMsgf(Callback.IsBound(), TEXT("ASSERT: [%i] %hs:\n'Callback' is not bound!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	FAsyncLoadGameFromSlotDelegate Delegate;
	Delegate.BindLambda([SafeCallback = PIESafeAsync::MakePIESafeCallback<USaveGame>(WorldContextObject, Callback)](const FString&, int32, USaveGame* LoadedSave)
	{
		SafeCallback(LoadedSave);
	});

	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, UserIndex, MoveTemp(Delegate));
}

// Completely removes given save data and creates new empty one
USaveGame* USaveUtilsLibrary::ResetSaveGameData(class USaveGame* SaveGame, const FString& SaveSlotName, int32 SaveSlotIndex)
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
bool USaveUtilsLibrary::CanSaveConfig(const UObject* Object)
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
void USaveUtilsLibrary::SaveConfig(UObject* Object)
{
	const bool bCanSaveConfig = CanSaveConfig(Object);
	if (!ensureMsgf(bCanSaveConfig, TEXT("ASSERT: [%i] %hs:\n'Object' can not be saved to the config file!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	Object->SaveConfig();
}
