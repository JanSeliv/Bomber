// Copyright (c) Yevhenii Selivanov.

#include "Bomber.h"
//---
#include "Kismet/GameplayStatics.h"
//---
#include "Modules/ModuleManager.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(Bomber)

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Bomber, "Bomber");

DEFINE_LOG_CATEGORY(LogBomber);

bool FTransientChecker::IsTransient(const UObject* Obj)
{
	static const FString TransientLevelName = TEXT("Transient");
	return !IsValid(Obj)
	       || !(Obj)->IsValidLowLevelFast()
	       || (Obj)->HasAllFlags(RF_ClassDefaultObject)
	       || UGameplayStatics::GetCurrentLevelName(Obj) == TransientLevelName;
}
