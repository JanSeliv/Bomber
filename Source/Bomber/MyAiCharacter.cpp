// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAiCharacter.h"

#include "Bomber.h"
#include "SingletonLibrary.h"

void AMyAiCharacter::UpdateAI_Implementation()
{
	AGeneratedMap* const LevelMap = USingletonLibrary::GetLevelMap(GetWorld());
	if (LevelMap == nullptr)  // Level Map is null
	{
		return;
	}

#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true)  // for editor only
	{
		UE_LOG_STR("PIE:UpdateAI: %s answered", this);
		AiMoveTo = FCell();
	}
#endif
}

///[AiDelegate]#if WITH_EDITOR
///[AiDelegate]void AMyAiCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
///[AiDelegate]{
///[AiDelegate]	if (IS_PIE(GetWorld()) == true)  ///[AiDelegate]For editor only
///[AiDelegate]	{
///[AiDelegate]		///[AiDelegate] Get the name of the property that was changed
///[AiDelegate]		const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
///[AiDelegate]
///[AiDelegate]		if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyAiCharacter, bShouldShowRenders))  ///[AiDelegate] Check the property name
///[AiDelegate]		{
///[AiDelegate]			UE_LOG(LogTemp, Warning, TEXT("PIE:PostEditChangeProperty: %s bShouldShowRenders: %s"), *this->GetName(), (bShouldShowRenders ? TEXT("true") : TEXT("false")));
///[AiDelegate]
///[AiDelegate]			///[AiDelegate]  Binding or unbinding render updates of render AI on creating\destroying elements
///[AiDelegate]			auto& Delegate = USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate;
///[AiDelegate]			if (bShouldShowRenders == true											   ///[AiDelegate] Is the render AI
///[AiDelegate]				&& Delegate.IsAlreadyBound(this, &AMyAiCharacter::UpdateAI) == false)  ///[AiDelegate] Is not bound
///[AiDelegate]			{
///[AiDelegate]				Delegate.AddDynamic(this, &AMyAiCharacter::UpdateAI);
///[AiDelegate]			}
///[AiDelegate]			else
///[AiDelegate]			{
///[AiDelegate]				Delegate.RemoveDynamic(this, &AMyAiCharacter::UpdateAI);
///[AiDelegate]			}
///[AiDelegate]		}
///[AiDelegate]	}
///[AiDelegate]
///[AiDelegate]	///[AiDelegate]Call the base class version
///[AiDelegate]	Super::PostEditChangeProperty(PropertyChangedEvent);
///[AiDelegate]}
///[AiDelegate]#endif  ///[AiDelegate]WITH_EDITOR