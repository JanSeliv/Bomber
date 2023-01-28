// Copyright (c) Yevhenii Selivanov

#include "MetaCheatManager.h"
//---
#include "Engine/Console.h"

// Returns the cheat command associated with specified CheatName meta value
const FMetaCheatCommand& UMetaCheatManager::GetCheatCommandByCheatName(const FName& CheatName) const
{
	for (const FMetaCheatCommand& CheatCommandIt : AllCheatCommands)
	{
		if (CheatCommandIt.CheatName.IsEqual(CheatName))
		{
			return CheatCommandIt;
		}
	}

	return FMetaCheatCommand::EmptyCommand;
}

// Is overridden to initialize all cheat commands on editor startup
void UMetaCheatManager::PostInitProperties()
{
	Super::PostInitProperties();

	InitAllCheatCommands();
}

// Called when CheatManager is created to allow any needed initialization
void UMetaCheatManager::InitCheatManager()
{
	Super::InitCheatManager();

	if (!UConsole::RegisterConsoleAutoCompleteEntries.IsBoundToObject(this))
	{
		UConsole::RegisterConsoleAutoCompleteEntries.AddUObject(this, &ThisClass::RegisterAutoCompleteEntries);
	}
}

// Is overridden to convert meta CheatName Your.Cheat.Name to the function name YourCheatFunction whenever user enters the command
bool UMetaCheatManager::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	constexpr bool bUseEscape = true;
	const FString OriginalCmd = Cmd;
	FString CommandName = TEXT("");
	if (FParse::Token(/*InOut*/Cmd, /*Out*/CommandName, bUseEscape))
	{
		// CommandName: is the CheatName (Your.Cheat.Name)
		// Cmd: is the value (if any) that was passed to the cheat
		const FMetaCheatCommand& CheatCommand = GetCheatCommandByCheatName(*CommandName);
		if (CheatCommand.IsValid())
		{
			// Get the function name (YourCheatFunction) from the CheatName (Your.Cheat.Name)
			// and append it with the value that was passed to the cheat to process the call
			// YourFunctionCheat Value
			constexpr bool bForceCallWithNonExec = true;
			const FString CmdString = CheatCommand.FunctionName.ToString() + Cmd;
			return CallFunctionByNameWithArguments(*CmdString, Ar, Executor, bForceCallWithNonExec);
		}
	}

	return Super::ProcessConsoleExec(*OriginalCmd, Ar, Executor);
}

// Garbage things before destroying the Cheat Manager
void UMetaCheatManager::BeginDestroy()
{
	UConsole::RegisterConsoleAutoCompleteEntries.RemoveAll(this);

	Super::BeginDestroy();
}

// Is bound to return all initialized meta cheat commands to see them in the console
void UMetaCheatManager::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand>& Commands) const
{
	for (const FMetaCheatCommand& CheatCommandIt : AllCheatCommands)
	{
		Commands.Emplace(CheatCommandIt.ToAutoCompleteCommand());
	}
}

// Finds and saves all cheat commands marked with 'CheatName' metadata
void UMetaCheatManager::InitAllCheatCommands()
{
#if WITH_EDITOR
	// It automatically adds DefaultMetaCheatManager.ini config on the editor startup to the Config folder on your project
	// to have your cheat commands with custom Cheat Names in the packaged build as well, you don't need to do anything specific about it.
	// Such solution is used because any metadata can be obtained only in the Editor, so we store it in the config file for the build.

	if (!HasAllFlags(RF_ClassDefaultObject))
	{
		// Do not init cheat commands for instances since we save them as default values into config file
		return;
	}

	if (!AllCheatCommands.IsEmpty())
	{
		AllCheatCommands.Empty();
	}

	// Find all cheat commands
	for (TFieldIterator<UFunction> FunctionIt(GetClass(), EFieldIteratorFlags::ExcludeSuper); FunctionIt; ++FunctionIt)
	{
		FMetaCheatCommand CheatCommand = FMetaCheatCommand::Create(*FunctionIt);
		if (CheatCommand.IsValid())
		{
			AllCheatCommands.Emplace(MoveTemp(CheatCommand));
		}
	}

	TryUpdateDefaultConfigFile();
#endif // WITH_EDITOR
}
