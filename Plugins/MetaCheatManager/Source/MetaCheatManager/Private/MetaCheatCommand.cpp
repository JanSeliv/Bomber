// Copyright (c) Yevhenii Selivanov

#include "MetaCheatCommand.h"
//---
#include "ConsoleSettings.h"

// Contains empty cheat command
const FMetaCheatCommand FMetaCheatCommand::EmptyCommand = FMetaCheatCommand();

// Returns the Auto Complete Command from this structure object
FAutoCompleteCommand FMetaCheatCommand::ToAutoCompleteCommand() const
{
	FAutoCompleteCommand AutoCompleteCommand;
	AutoCompleteCommand.Command = CheatName.ToString();
	AutoCompleteCommand.Desc = CheatDescription.ToString();
	static FColor AutoCompleteCommandColor = GetDefault<UConsoleSettings>()->AutoCompleteCommandColor;
	AutoCompleteCommand.Color = AutoCompleteCommandColor;
	return AutoCompleteCommand;
}

#if WITH_EDITOR
// Builds all the data for this command by function ptr
FMetaCheatCommand FMetaCheatCommand::Create(const UFunction* InFunction)
{
	if (!InFunction)
	{
		return EmptyCommand;
	}

	FMetaCheatCommand CheatCommand;
	CheatCommand.CheatName = FindCheatMetaData(InFunction);
	if (CheatCommand.CheatName.IsNone())
	{
		// No cheat name, so this is not a cheat command
		return EmptyCommand;
	}

	CheatCommand.FunctionName = InFunction->GetFName();
	CheatCommand.CheatDescription = FindCheatDescription(InFunction);
	return CheatCommand;
}

// Finds a value of the 'CheatName' meta data for the specified function, none if CheatName is not set
FName FMetaCheatCommand::FindCheatMetaData(const UFunction* InFunction)
{
	static const FName CheatNameMetaKey = TEXT("CheatName");
	checkf(InFunction, TEXT("%s: 'InFunction' is null"), *FString(__FUNCTION__));
	const FString* FoundMetaData = InFunction->FindMetaData(CheatNameMetaKey);
	return FoundMetaData ? FName(*FoundMetaData) : NAME_None;
}

// Finds the description for the specified function
FName FMetaCheatCommand::FindCheatDescription(const UFunction* InFunction)
{
	checkf(InFunction, TEXT("%s: 'InFunction' is null"), *FString(__FUNCTION__));

	FString Description = TEXT("");

	// Start with description of function param: Name[Type]. E.x: NewLevel[int32]
	for (TFieldIterator<FProperty> PropIt(InFunction); PropIt && PropIt->PropertyFlags & CPF_Parm; ++PropIt)
	{
		if (const FProperty* Prop = *PropIt)
		{
			Description += FString::Printf(TEXT("%s[%s] "), *Prop->GetName(), *Prop->GetCPPType());
		}
	}

	// Get the first line of the function tooltip
	static const FString Delimiter = TEXT("\n");
	FString FunctionToolTip = InFunction->GetToolTipText().ToString();
	TArray<FString> SeparatedStrings;
	FunctionToolTip.ParseIntoArray(SeparatedStrings, *Delimiter);
	if (!FunctionToolTip.IsEmpty())
	{
		FunctionToolTip = SeparatedStrings[0];
	}

	// Add function tooltip to the total description
	Description.Append(FunctionToolTip);
	return FName(*Description);
}
#endif // WITH_EDITOR
