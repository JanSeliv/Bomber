// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/BmrPowerupWidget.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Actors/BmrPawn.h"
#include "DalSubsystem.h"
#include "DataAssets/BmrUIDataAsset.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/Image.h"
#include "Components/RadialSlider.h"
#include "Engine/Texture2D.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupWidget)

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Updates the blends slider target to which widget will interpolate
void UBmrPowerupWidget::SetTargetValue(float NewValue, float MaxValue, bool bImmediateUpdate)
{
	NewValue = FMath::Max(NewValue, 0.f);
	MaxValue = FMath::Max(MaxValue, NewValue);
	TargetValue = NewValue / MaxValue;

	if (bImmediateUpdate)
	{
		checkf(RadialSlider, TEXT("ERROR: [%i] %hs:\n'RadialSlider' is null!"), __LINE__, __FUNCTION__);
		RadialSlider->SetValue(TargetValue);
	}
	else
	{
		// Start the blend
		bNeedsUpdate = true;
		ElapsedLerpTime = 0.f;
	}
}

// Updates the icon of the powerup powerup in the UI
void UBmrPowerupWidget::SetPowerupIcon(FBmrPowerupTag InPowerupTag)
{
	if (!ensureMsgf(PowerUpIcon, TEXT("ASSERT: [%i] %hs:\n'PowerUpIcon' is not constructed!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	PowerupTag = InPowerupTag;

	UTexture2D* IconTexture = UBmrUIDataAsset::Get().GetPowerupIcon(InPowerupTag);
	ensureMsgf(PowerUpIcon, TEXT("ASSERT: [%i] %hs:\n'PowerUpIcon' is not set in UI Data Asset"), __LINE__, __FUNCTION__);
	PowerUpIcon->SetBrushResourceObject(IconTexture);
}

// Called before the underlying slate widget is constructed to update widget at design time
void UBmrPowerupWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	UDalSubsystem::Get().ListenForDataAsset<UBmrUIDataAsset>(this, [this](const UBmrUIDataAsset& DataAsset)
	{
		// Update the icon brush in the editor when tag property is changed
		SetPowerupIcon(PowerupTag);
	});
}

// Called after the underlying slate widget is constructed
void UBmrPowerupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensureMsgf(PowerupTag != FBmrPowerupTag::None, TEXT("ASSERT: [%i] %hs:\n'PowerupTag' is not set!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::Player_LocalPawnReady, this, &ThisClass::OnLocalPawnReady);
}

// Is executed every tick when widget is enabled
void UBmrPowerupWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bNeedsUpdate
	    || LerpDuration <= 0.f)
	{
		return;
	}

	const float Current = RadialSlider->GetValue();
	ElapsedLerpTime += InDeltaTime;

	const float Alpha = FMath::Clamp(ElapsedLerpTime / LerpDuration, 0.f, 1.f);
	const float NewValue = FMath::Lerp(Current, TargetValue, Alpha);

	RadialSlider->SetValue(NewValue);

	if (FMath::IsNearlyEqual(NewValue, TargetValue, KINDA_SMALL_NUMBER)
	    || Alpha >= 1.f)
	{
		RadialSlider->SetValue(TargetValue);
		bNeedsUpdate = false;
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the local player state is initialized and its assigned character is ready
void UBmrPowerupWidget::OnLocalPawnReady_Implementation(const FGameplayEventData& Payload)
{
	const ABmrPawn* Character = Cast<ABmrPawn>(Payload.Instigator.Get());
	checkf(Character, TEXT("ERROR: [%i] %hs:\n'Character' is null!"), __LINE__, __FUNCTION__);

	UAbilitySystemComponent& ASC = Character->GetAbilitySystemComponentChecked();
	const FGameplayAttribute PowerupAttribute = UBmrPowerupsAttributeSet::Conv_TagToBaseAttribute(PowerupTag);
	ASC.GetGameplayAttributeValueChangeDelegate(PowerupAttribute).AddUObject(this, &ThisClass::OnPowerupAttributeChanged);

	constexpr bool bImmediateUpdate = true;
	const UBmrPowerupsAttributeSet& PowerupsAttributeSet = UBmrPowerupsAttributeSet::Get(&ASC);
	const float InitialValue = PowerupsAttributeSet.GetPowerupValueByTag(PowerupTag);
	const float MaxValue = PowerupsAttributeSet.GetPowerupMaxValueByTag(PowerupTag);
	SetTargetValue(InitialValue, MaxValue, bImmediateUpdate);
}

// Is called when the associated powerup attribute is changed
void UBmrPowerupWidget::OnPowerupAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	const UAbilitySystemComponent* ASC = OnAttributeChangeData.GEModData ? &OnAttributeChangeData.GEModData->Target : UBmrBlueprintFunctionLibrary::GetLocalAbilitySystemComponent();
	const UBmrPowerupsAttributeSet& PowerupsAttributeSet = UBmrPowerupsAttributeSet::Get(ASC);
	const float MaxValue = PowerupsAttributeSet.GetPowerupMaxValueByTag(PowerupTag);
	SetTargetValue(OnAttributeChangeData.NewValue, MaxValue);
}
