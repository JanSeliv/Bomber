// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_MyBaseViewModel)

// Returns the world where this View Model is created
UWorld* UMVVM_MyBaseViewModel::GetWorld() const
{
	return UUtilsLibrary::GetPlayWorld();
}

// If false, the View Model will not be constructed
bool UMVVM_MyBaseViewModel::CanConstructViewModel_Implementation() const
{
	// Is not abstract class
	return !GetClass()->HasAnyClassFlags(CLASS_Abstract);
}
