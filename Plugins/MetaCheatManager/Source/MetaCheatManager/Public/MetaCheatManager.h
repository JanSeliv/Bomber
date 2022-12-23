// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/CheatManager.h"
//---
#include "MetaCheatCommand.h"
//---
#include "MetaCheatManager.generated.h"

/**
 * Allows customize command names by 'CheatName =' meta for the functions in the Meta Cheat Manager subclass.
 *
 * Only two next steps are required to see your function with custom name in console:
 * 1. Inherit from this class instead of 'UCheatManager' to use this feature.
 * 2. Add 'CheatName =' meta to the functions in your subclass to customize command names like on example below:
 * 	UFUNCTION(meta = (CheatName = "Your.Cheat.Name"))
 *	void YourCheatFunction();
 *
 * Notice#1: 'Exec' specifier is not required anymore for such functions in your subclass.
 * Notice#2: it works even without creating any blueprint, so only your code class would be enough, but you can have BP if you want.
 * Notice#3: It automatically creates the DefaultMetaCheatManager.ini config on the editor startup to the Config folder of your project
 * to have your cheat commands with custom Cheat Names in the packaged build as well, you don't need to do anything specific about it.
 * Such solution is used because any metadata can be obtained only in the Editor, so we store it in the config file for the build.
 */
UCLASS(Config = "MetaCheatManager", DefaultConfig)
class METACHEATMANAGER_API UMetaCheatManager : public UCheatManager
{
	GENERATED_BODY()

	/** Returns all cheat commands exposed by this cheat manager.
	 * @see UMetaCheatManager::AllCheatCommandsInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE TArray<FMetaCheatCommand>& GetAllCheatCommands() const { return AllCheatCommands; }

	/** Returns the cheat command associated with specified CheatName meta value. */
	UFUNCTION(BlueprintPure, Category = "C++")
	virtual const FMetaCheatCommand& GetCheatCommandByCheatName(const FName& CheatName) const;

protected:
	/** Contains all cheat commands exposed by this cheat manager.
	 * Is automatically saved into config file while in editor to have these commands available in builds where is no access to meta data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "All Cheat Commands"))
	TArray<FMetaCheatCommand> AllCheatCommands;

	/** Is overridden to initialize all cheat commands on editor startup. */
	virtual void PostInitProperties() override;

	/** Called when CheatManager is created to allow any needed initialization. */
	virtual void InitCheatManager() override;

	/** Is overridden to convert a meta CheatName 'Your.Cheat.Name'
	 * to the function name 'YourCheatFunction' to process the call whenever user enters the command. */
	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;

	/** Garbage things before destroying the Cheat Manager. */
	virtual void BeginDestroy() override;

	/** Is bound to return all initialized meta cheat commands to see them in the console. */
	virtual void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand>& Commands) const;

	/** Finds and saves all cheat commands marked with 'CheatName' metadata.
	 * @warning its implementation is editor-only
	 * since we don't have access to any meta data in builds,
	 * but you can override it to use your own implementation.
	 * @see UMetaCheatManager::AllCheatCommandsInternal */
	virtual void InitAllCheatCommands();
};
