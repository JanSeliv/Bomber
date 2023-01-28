// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"
#include "MetaCheatCommand.generated.h"

struct FAutoCompleteCommand;

/**
 * Describes a cheat command.
 */
USTRUCT(BlueprintType)
struct METACHEATMANAGER_API FMetaCheatCommand
{
	GENERATED_BODY()

	/** Contains empty cheat command. */
	static const FMetaCheatCommand EmptyCommand;

	/** The name of command that will be displayed in console. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CheatName = NAME_None;

	/** The original name of function. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FunctionName = NAME_None;

	/** The description of the command. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CheatDescription = NAME_None;

	/** Returns true if this command contains correct data */
	FORCEINLINE bool IsValid() const
	{
		return CheatName != NAME_None
			&& CheatDescription != NAME_None
			&& FunctionName != NAME_None;
	}

	/** Returns the Auto Complete Command from this structure object. */
	FAutoCompleteCommand ToAutoCompleteCommand() const;

#if WITH_EDITOR
	/** Builds all the data for this command by function ptr.
	 * Is editor-only since the meta data is not available in the build.*/
	static FMetaCheatCommand Create(const UFunction* InFunction);

	/** Finds a value of the 'CheatName' meta data for the specified function, none if CheatName is not set.
	 * Is editor-only since the meta data is not available in the build.*/
	static FName FindCheatMetaData(const UFunction* InFunction);

	/** Finds the description for the specified function.
	 * Is editor-only since the meta data is not available in the build.*/
	static FName FindCheatDescription(const UFunction* InFunction);
#endif // WITH_EDITOR
};
