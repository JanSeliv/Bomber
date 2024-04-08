// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_MyBaseViewModel)

// If false, the View Model will not be constructed
bool UMVVM_MyBaseViewModel::CanConstructViewModel_Implementation() const
{
	// Is not abstract class
	return !GetClass()->HasAnyClassFlags(CLASS_Abstract);
}
