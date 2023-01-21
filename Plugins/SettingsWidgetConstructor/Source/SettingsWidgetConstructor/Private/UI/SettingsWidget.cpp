// Copyright (c) Yevhenii Selivanov

#include "UI/SettingsWidget.h"
//---
#include "GameFramework/GameUserSettings.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SizeBox.h"
//---
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Data/SettingsDataAsset.h"
#include "Data/SettingsDataTable.h"
#include "UI/SettingSubWidget.h"

// Try to find the setting row
const FSettingsPicker& USettingsWidget::FindSettingRow(FName PotentialTagName) const
{
	if (PotentialTagName.IsNone())
	{
		return FSettingsPicker::Empty;
	}

	const FSettingsPicker* FoundRow = &FSettingsPicker::Empty;

	// Find row by specified substring
	const FString TagSubString(PotentialTagName.ToString());
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FString TagStringIt(RowIt.Key.ToString());
		if (TagStringIt.Contains(TagSubString))
		{
			FoundRow = &RowIt.Value;
			break;
		}
	}

	return *FoundRow;
}

// Returns the found row by specified tag
const FSettingsPicker& USettingsWidget::GetSettingRow(const FSettingTag& SettingTag) const
{
	if (!SettingTag.IsValid())
	{
		return FSettingsPicker::Empty;
	}

	const FSettingsPicker* FoundRow = SettingsTableRowsInternal.Find(SettingTag.GetTagName());
	return FoundRow ? *FoundRow : FSettingsPicker::Empty;
}

// Save all settings into their configs
void USettingsWidget::SaveSettings()
{
	ApplySettings();

	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		if (UObject* ContextObject = RowIt.Value.PrimaryData.StaticContextObject.Get())
		{
			ContextObject->SaveConfig();
		}
	}
}

// Apply all current settings on device
void USettingsWidget::ApplySettings()
{
	UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings();
	if (!GameUserSettings)
	{
		return;
	}

	constexpr bool bCheckForCommandLineOverrides = false;
	GameUserSettings->ApplySettings(bCheckForCommandLineOverrides);
}

// Update settings on UI
void USettingsWidget::UpdateSettings(const FGameplayTagContainer& SettingsToUpdate)
{
	if (SettingsToUpdate.IsEmpty()
		|| !SettingsToUpdate.IsValidIndex(0))
	{
		return;
	}

	if (SettingsTableRowsInternal.IsEmpty())
	{
		UpdateSettingsTableRows();
	}

	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPicker& Setting = RowIt.Value;
		const FSettingTag& SettingTag = Setting.PrimaryData.Tag;
		if (!SettingTag.MatchesAny(SettingsToUpdate))
		{
			continue;
		}

		const FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
		if (!ChosenData)
		{
			continue;
		}

		if (ChosenData == &Setting.Checkbox)
		{
			const bool NewValue = GetCheckboxValue(SettingTag);
			SetSettingCheckbox(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.Combobox)
		{
			const int32 NewValue = GetComboboxIndex(SettingTag);
			SetSettingComboboxIndex(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.Slider)
		{
			const double NewValue = GetSliderValue(SettingTag);
			SetSettingSlider(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.TextLine)
		{
			FText NewValue = TEXT_NONE;
			GetTextLineValue(SettingTag, /*Out*/NewValue);
			SetSettingTextLine(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.UserInput)
		{
			const FName NewValue = GetUserInputValue(SettingTag);
			SetSettingUserInput(SettingTag, NewValue);
		}
	}
}

// Returns the name of found tag by specified function
const FSettingTag& USettingsWidget::GetTagByFunction(const FSettingFunctionPicker& FunctionPicker) const
{
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPrimary& PrimaryData = RowIt.Value.PrimaryData;
		if (PrimaryData.Getter == FunctionPicker
			|| PrimaryData.Setter == FunctionPicker)
		{
			return PrimaryData.Tag;
		}
	}

	return FSettingTag::EmptySettingTag;
}

// Set value to the option by tag
void USettingsWidget::SetSettingValue(FName TagName, const FString& Value)
{
	const FSettingsPicker& FoundRow = FindSettingRow(TagName);
	if (!FoundRow.IsValid())
	{
		return;
	}

	const FSettingsDataBase* ChosenData = FoundRow.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	const FSettingTag& Tag = FoundRow.PrimaryData.Tag;
	if (!Tag.IsValid())
	{
		return;
	}

	if (ChosenData == &FoundRow.Button)
	{
		SetSettingButtonPressed(Tag);
	}
	else if (ChosenData == &FoundRow.Checkbox)
	{
		const bool NewValue = Value.ToBool();
		SetSettingCheckbox(Tag, NewValue);
	}
	else if (ChosenData == &FoundRow.Combobox)
	{
		if (Value.IsNumeric())
		{
			const int32 NewValue = FCString::Atoi(*Value);
			SetSettingComboboxIndex(Tag, NewValue);
		}
		else
		{
			static const FString Delimiter = TEXT(",");
			TArray<FString> SeparatedStrings;
			Value.ParseIntoArray(SeparatedStrings, *Delimiter);

			TArray<FText> NewMembers;
			NewMembers.Reserve(SeparatedStrings.Num());
			for (FString& StringIt : SeparatedStrings)
			{
				NewMembers.Emplace(FText::FromString(MoveTemp(StringIt)));
			}
			SetSettingComboboxMembers(Tag, NewMembers);
		}
	}
	else if (ChosenData == &FoundRow.Slider)
	{
		const double NewValue = FCString::Atod(*Value);
		SetSettingSlider(Tag, NewValue);
	}
	else if (ChosenData == &FoundRow.TextLine)
	{
		const FText NewValue = FText::FromString(Value);
		SetSettingTextLine(Tag, NewValue);
	}
	else if (ChosenData == &FoundRow.UserInput)
	{
		const FName NewValue = *Value;
		SetSettingUserInput(Tag, NewValue);
	}
}

// Press button
void USettingsWidget::SetSettingButtonPressed(const FSettingTag& ButtonTag)
{
	if (!ButtonTag.IsValid())
	{
		return;
	}

	const FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ButtonTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	SettingsRowPtr->Button.OnButtonPressed.ExecuteIfBound();

	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	PlayUIClickSFX();
}

// Toggle checkbox
void USettingsWidget::SetSettingCheckbox(const FSettingTag& CheckboxTag, bool InValue)
{
	if (!CheckboxTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(CheckboxTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	bool& bIsSetRef = SettingsRowPtr->Checkbox.bIsSet;
	if (bIsSetRef == InValue)
	{
		return;
	}

	bIsSetRef = InValue;
	SettingsRowPtr->Checkbox.OnSetterBool.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetCheckbox(CheckboxTag, InValue);
	PlayUIClickSFX();
}

// Set chosen member index for a combobox
void USettingsWidget::SetSettingComboboxIndex(const FSettingTag& ComboboxTag, int32 InValue)
{
	if (!ComboboxTag.IsValid())
	{
		return;
	}

	if (InValue == INDEX_NONE)
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ComboboxTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	FSettingsPicker& SettingsRowRef = *SettingsRowPtr;
	int32& ChosenMemberIndexRef = SettingsRowRef.Combobox.ChosenMemberIndex;
	if (ChosenMemberIndexRef == InValue)
	{
		return;
	}

	ChosenMemberIndexRef = InValue;
	SettingsRowRef.Combobox.OnSetterInt.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowRef.PrimaryData.SettingsToUpdate);

	// BP implementation
	SetComboboxIndex(ComboboxTag, InValue);
}

// Set new members for a combobox
void USettingsWidget::SetSettingComboboxMembers(const FSettingTag& ComboboxTag, const TArray<FText>& InValue)
{
	if (!ComboboxTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ComboboxTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	SettingsRowPtr->Combobox.Members = InValue;
	SettingsRowPtr->Combobox.OnSetMembers.ExecuteIfBound(InValue);

	// BP implementation
	SetComboboxMembers(ComboboxTag, InValue);
}

// Set current value for a slider
void USettingsWidget::SetSettingSlider(const FSettingTag& SliderTag, double InValue)
{
	if (!SliderTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(SliderTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	static constexpr double MinValue = 0.0;
	static constexpr float MaxValue = 1.0;
	const double NewValue = FMath::Clamp(InValue, MinValue, MaxValue);
	double& ChosenValueRef = SettingsRowPtr->Slider.ChosenValue;
	if (ChosenValueRef == NewValue)
	{
		return;
	}

	ChosenValueRef = NewValue;
	SettingsRowPtr->Slider.OnSetterFloat.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetSlider(SliderTag, InValue);
}

// Set new text
void USettingsWidget::SetSettingTextLine(const FSettingTag& TextLineTag, const FText& InValue)
{
	if (!TextLineTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(TextLineTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	FSettingsPrimary& PrimaryRef = SettingsRowPtr->PrimaryData;
	FText& CaptionRef = PrimaryRef.Caption;
	if (CaptionRef.EqualTo(InValue))
	{
		return;
	}

	CaptionRef = InValue;
	SettingsRowPtr->TextLine.OnSetterText.ExecuteIfBound(InValue);
	UpdateSettings(PrimaryRef.SettingsToUpdate);

	if (USettingTextLine* SettingTextLine = Cast<USettingTextLine>(PrimaryRef.SettingSubWidget))
	{
		SettingTextLine->SetCaptionText(InValue);
	}
}

// Set new text for an input box
void USettingsWidget::SetSettingUserInput(const FSettingTag& UserInputTag, FName InValue)
{
	if (!UserInputTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(UserInputTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	FSettingsUserInput& UserInputRef = SettingsRowPtr->UserInput;
	if (UserInputRef.UserInput.IsEqual(InValue)
		|| InValue.IsNone())
	{
		// Is not needed to update
		return;
	}

	if (UserInputRef.MaxCharactersNumber > 0)
	{
		// Limit the length of the string
		const FString NewValueStr = InValue.ToString().Left(UserInputRef.MaxCharactersNumber);
		InValue = *NewValueStr;

		if (USettingUserInput* SettingUserInput = Cast<USettingUserInput>(SettingsRowPtr->PrimaryData.SettingSubWidget.Get()))
		{
			SettingUserInput->SetEditableText(FText::FromString(NewValueStr));
		}
	}

	UserInputRef.UserInput = InValue;
	UserInputRef.OnSetterName.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetUserInput(UserInputTag, InValue);
	PlayUIClickSFX();
}

// Set new custom widget for setting by specified tag
void USettingsWidget::SetSettingCustomWidget(const FSettingTag& CustomWidgetTag, USettingCustomWidget* InCustomWidget)
{
	if (!CustomWidgetTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(CustomWidgetTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	TWeakObjectPtr<USettingSubWidget>& CustomWidgetRef = SettingsRowPtr->PrimaryData.SettingSubWidget;
	if (CustomWidgetRef == InCustomWidget)
	{
		return;
	}

	CustomWidgetRef.Reset();
	CustomWidgetRef = InCustomWidget;
	SettingsRowPtr->CustomWidget.OnSetterWidget.ExecuteIfBound(InCustomWidget);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);
}

// Returns is a checkbox toggled
bool USettingsWidget::GetCheckboxValue(const FSettingTag& CheckboxTag) const
{
	if (!CheckboxTag.IsValid())
	{
		return false;
	}

	const FSettingsPicker& FoundRow = GetSettingRow(CheckboxTag);
	bool Value = false;
	if (FoundRow.IsValid())
	{
		const FSettingsCheckbox& Data = FoundRow.Checkbox;
		Value = Data.bIsSet;

		const USettingFunctionTemplate::FOnGetterBool& Getter = Data.OnGetterBool;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Returns chosen member index of a combobox
int32 USettingsWidget::GetComboboxIndex(const FSettingTag& ComboboxTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(ComboboxTag);
	int32 Value = false;
	if (FoundRow.IsValid())
	{
		const FSettingsCombobox& Data = FoundRow.Combobox;
		Value = Data.ChosenMemberIndex;

		const USettingFunctionTemplate::FOnGetterInt& Getter = Data.OnGetterInt;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Get all members of a combobox
void USettingsWidget::GetComboboxMembers(const FSettingTag& ComboboxTag, TArray<FText>& OutMembers) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(ComboboxTag);
	if (FoundRow.IsValid())
	{
		const FSettingsCombobox& Data = FoundRow.Combobox;
		OutMembers = Data.Members;

		const USettingFunctionTemplate::FOnGetMembers& Getter = Data.OnGetMembers;
		if (Getter.IsBound())
		{
			Getter.Execute(OutMembers);
		}
	}
}

// Get current value of a slider [0...1]
double USettingsWidget::GetSliderValue(const FSettingTag& SliderTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(SliderTag);
	double Value = 0.0;
	if (FoundRow.IsValid())
	{
		const FSettingsSlider& Data = FoundRow.Slider;
		Value = Data.ChosenValue;

		const USettingFunctionTemplate::FOnGetterFloat& Getter = Data.OnGetterFloat;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Get current text of a simple text widget
void USettingsWidget::GetTextLineValue(const FSettingTag& TextLineTag, FText& OutText) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(TextLineTag);
	if (FoundRow.IsValid())
	{
		OutText = FoundRow.PrimaryData.Caption;

		const USettingFunctionTemplate::FOnGetterText& Getter = FoundRow.TextLine.OnGetterText;
		if (Getter.IsBound())
		{
			Getter.Execute(OutText);
		}
	}
}

// Get current input name of the text input
FName USettingsWidget::GetUserInputValue(const FSettingTag& UserInputTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(UserInputTag);
	FName Value = NAME_None;
	if (FoundRow.IsValid())
	{
		const FSettingsUserInput& Data = FoundRow.UserInput;
		Value = Data.UserInput;

		const USettingFunctionTemplate::FOnGetterName& Getter = Data.OnGetterName;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Get custom widget of the setting by specified tag
USettingCustomWidget* USettingsWidget::GetCustomWidget(const FSettingTag& CustomWidgetTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(CustomWidgetTag);
	USettingCustomWidget* CustomWidget = nullptr;
	if (FoundRow.IsValid())
	{
		CustomWidget = Cast<USettingCustomWidget>(FoundRow.PrimaryData.SettingSubWidget.Get());

		const USettingFunctionTemplate::FOnGetterWidget& Getter = FoundRow.CustomWidget.OnGetterWidget;
		if (Getter.IsBound())
		{
			CustomWidget = Getter.Execute();
		}
	}
	return CustomWidget;
}

// Get setting widget object by specified tag
USettingSubWidget* USettingsWidget::GetSettingSubWidget(const FSettingTag& SettingTag) const
{
	const FSettingsPrimary& PrimaryData = GetSettingRow(SettingTag).PrimaryData;
	return PrimaryData.IsValid() ? PrimaryData.SettingSubWidget.Get() : nullptr;
}

// Returns the size of the Settings widget on the screen
FVector2D USettingsWidget::GetSettingsSize() const
{
	const FVector2D PercentSize = USettingsDataAsset::Get().GetSettingsPercentSize();

	UObject* WorldContextObject = GetOwningPlayer();
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(WorldContextObject);
	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(WorldContextObject);

	const FVector2D NewViewportSize = ViewportSize * PercentSize;
	return ViewportScale ? NewViewportSize / ViewportScale : NewViewportSize;
}

// Returns the size of specified category on the screen
FVector2D USettingsWidget::GetSubWidgetsSize(int32 SectionsBitmask) const
{
	if (!SectionsBitmask)
	{
		return FVector2D::ZeroVector;
	}

	TArray<const UWidget*> DesiredWidgets;

	constexpr int32 HeaderAlignment = static_cast<int32>(EMyVerticalAlignment::Header);
	if (HeaderAlignment & SectionsBitmask)
	{
		DesiredWidgets.Emplace(HeaderVerticalBox);
	}

	constexpr int32 ContentAlignment = static_cast<int32>(EMyVerticalAlignment::Content);
	if (ContentAlignment & SectionsBitmask)
	{
		DesiredWidgets.Emplace(ContentHorizontalBox);
	}

	constexpr int32 FooterAlignment = static_cast<int32>(EMyVerticalAlignment::Footer);
	if (FooterAlignment & SectionsBitmask)
	{
		DesiredWidgets.Emplace(FooterVerticalBox);
	}

	FVector2D SubWidgetsHeight = FVector2D::ZeroVector;
	for (const UWidget* DesiredWidgetIt : DesiredWidgets)
	{
		if (DesiredWidgetIt)
		{
			const FVector2D SubWidgetHeight = DesiredWidgetIt->GetDesiredSize();
			ensureAlwaysMsgf(!SubWidgetHeight.IsZero(), TEXT("ASSERT: 'SubWidgetHeight' is zero, can't get the size of subwidget, most likely widget is not initialized yet, call ForceLayoutPrepass()"));
			SubWidgetsHeight += SubWidgetHeight;
		}
	}
	return SubWidgetsHeight;
}


// Returns the height of a setting scrollbox on the screen
float USettingsWidget::GetScrollBoxHeight() const
{
	const USettingsDataAsset& SettingsData = USettingsDataAsset::Get();

	// The widget size
	const FVector2D SettingsSize = GetSettingsSize();

	// Margin size
	constexpr int32 Margins = static_cast<int32>(EMyVerticalAlignment::Margins);
	const FVector2D MarginsSize = GetSubWidgetsSize(Margins);

	// Additional padding sizes
	float Paddings = 0.f;
	const FMargin SettingsPadding = SettingsData.GetSettingsPadding();
	Paddings += SettingsPadding.Top + SettingsPadding.Bottom;
	const FMargin ScrollBoxPadding = SettingsData.GetScrollboxPadding();
	Paddings += ScrollBoxPadding.Top + ScrollBoxPadding.Bottom;

	const float ScrollBoxHeight = (SettingsSize - MarginsSize).Y - Paddings;

	// Scale scrollbox
	const float PercentSize = FMath::Clamp(SettingsData.GetScrollboxPercentHeight(), 0.f, 1.f);
	const float ScaledHeight = ScrollBoxHeight * PercentSize;

	return ScaledHeight;
}

// Is blueprint-event called that returns the style brush by specified button state
FSlateBrush USettingsWidget::GetButtonBrush(ESettingsButtonState State) const
{
	const USettingsDataAsset& SettingsDataAsset = USettingsDataAsset::Get();
	const FMiscThemeData& MiscThemeData = SettingsDataAsset.GetMiscThemeData();
	const FButtonThemeData& ButtonThemeData = SettingsDataAsset.GetButtonThemeData();

	FSlateColor SlateColor;
	switch (State)
	{
	case ESettingsButtonState::Normal:
		SlateColor = MiscThemeData.ThemeColorNormal;
		break;
	case ESettingsButtonState::Hovered:
		SlateColor = MiscThemeData.ThemeColorHover;
		break;
	case ESettingsButtonState::Pressed:
		SlateColor = MiscThemeData.ThemeColorExtra;
		break;
	default:
		SlateColor = FLinearColor::White;
	}

	FSlateBrush SlateBrush;
	SlateBrush.TintColor = SlateColor;
	SlateBrush.DrawAs = ButtonThemeData.DrawAs;
	SlateBrush.Margin = ButtonThemeData.Margin;
	SlateBrush.SetImageSize(ButtonThemeData.Size);
	SlateBrush.SetResourceObject(ButtonThemeData.Texture);

	return SlateBrush;
}

// Called after the underlying slate widget is constructed
void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateSettingsTableRows();

	TryConstructSettings();
}

// Constructs settings if viewport is ready otherwise Wait until viewport become initialized
void USettingsWidget::TryConstructSettings()
{
	if (UUtilsLibrary::IsViewportInitialized())
	{
		ConstructSettings();
	}
	else if (!FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewportResizedWhenInit);
	}
}

// Is called right after the game was started and windows size is set to construct settings
void USettingsWidget::OnViewportResizedWhenInit(FViewport* Viewport, uint32 Index)
{
	if (FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}

	ConstructSettings();
}

// Construct all settings from the settings data table
void USettingsWidget::ConstructSettings()
{
	// BP implementation to cache some data before creating subwidgets
	OnConstructSettings();

	for (TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		AddSetting(RowIt.Value);
	}

	UpdateScrollBoxesHeight();
}

void USettingsWidget::UpdateSettingsTableRows()
{
	const USettingsDataTable* SettingsDataTable = USettingsDataAsset::Get().GetSettingsDataTable();
	if (!ensureMsgf(SettingsDataTable, TEXT("ASSERT: 'SettingsDataTable' is not valid")))
	{
		return;
	}

	TMap<FName, FSettingsRow> SettingRows;
	SettingsDataTable->GetSettingRows(/*Out*/SettingRows);
	if (!ensureMsgf(!SettingRows.IsEmpty(), TEXT("ASSERT: 'SettingRows' are empty")))
	{
		return;
	}

	// Reset values if currently are set
	OverallColumnsNumInternal = 1;
	SettingsTableRowsInternal.Empty();

	SettingsTableRowsInternal.Reserve(SettingRows.Num());
	for (const TTuple<FName, FSettingsRow>& SettingRowIt : SettingRows)
	{
		const FSettingsPicker& SettingsPicker = SettingRowIt.Value.SettingsPicker;
		SettingsTableRowsInternal.Emplace(SettingRowIt.Key, SettingsPicker);

		// Set overall columns num by amount of rows that are marked to be started on next column
		OverallColumnsNumInternal += static_cast<int32>(SettingsPicker.PrimaryData.bStartOnNextColumn);
	}
}

// Is called when In-Game menu became opened or closed
void USettingsWidget::OnToggleSettings(bool bIsVisible)
{
	PlayUIClickSFX();

	if (OnToggledSettings.IsBound())
	{
		OnToggledSettings.Broadcast(bIsVisible);
	}
}

// Bind and set static object delegate
void USettingsWidget::TryBindStaticContext(FSettingsPrimary& Primary)
{
	UObject* FoundContextObj = nullptr;
	if (UFunction* FunctionPtr = Primary.StaticContext.GetFunction())
	{
		FunctionPtr->ProcessEvent(FunctionPtr, /*Out*/&FoundContextObj);
	}

	if (!FoundContextObj)
	{
		return;
	}

	Primary.StaticContextObject = FoundContextObj;

	const UClass* ContextClass = FoundContextObj->GetClass();
	if (!ensureMsgf(ContextClass, TEXT("ASSERT: 'ContextClass' is not valid")))
	{
		return;
	}

	// Cache all functions that are contained in returned object
	for (TFieldIterator<UFunction> It(ContextClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		const UFunction* FunctionIt = *It;
		if (!FunctionIt)
		{
			continue;
		}

		const FName FunctionNameIt = FunctionIt->GetFName();
		if (!FunctionNameIt.IsNone())
		{
			Primary.StaticContextFunctionList.Emplace(FunctionNameIt);
		}
	}
}

// Creates new widget based on specified setting class and sets it to specified primary data
USettingSubWidget* USettingsWidget::CreateSettingSubWidget(FSettingsPrimary& InOutPrimary, const TSubclassOf<USettingSubWidget> SettingSubWidgetClass)
{
	if (!SettingSubWidgetClass)
	{
		return nullptr;
	}

	USettingSubWidget* SettingSubWidget = CreateWidget<USettingSubWidget>(this, SettingSubWidgetClass);
	InOutPrimary.SettingSubWidget = SettingSubWidget;
	SettingSubWidget->SetSettingsWidget(this);
	SettingSubWidget->SetSettingPrimaryRow(InOutPrimary);
	SettingSubWidget->SetLineHeight(InOutPrimary.LineHeight);
	SettingSubWidget->SetCaptionText(InOutPrimary.Caption);

	return SettingSubWidget;
}

// Starts adding settings on the next column
void USettingsWidget::StartNextColumn_Implementation()
{
	// BP implementation
	// ...
}

// Automatically sets the height for all scrollboxes in the Settings
void USettingsWidget::UpdateScrollBoxesHeight()
{
	ForceLayoutPrepass(); // Call it to make GetSettingsHeight work since it is called during widget construction
	const float ScrollBoxHeight = GetScrollBoxHeight();

	for (const USettingScrollBox* ScrollBoxIt : SettingScrollBoxesInternal)
	{
		USizeBox* SizeBoxWidget = ScrollBoxIt ? ScrollBoxIt->GetSizeBoxWidget() : nullptr;
		if (SizeBoxWidget)
		{
			SizeBoxWidget->SetMaxDesiredHeight(ScrollBoxHeight);
		}
	}
}

// Display settings on UI
void USettingsWidget::OpenSettings()
{
	if (IsVisible())
	{
		// Is already shown
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	OnToggleSettings(true);

	OnOpenSettings();
}

// Save and close the settings widget
void USettingsWidget::CloseSettings()
{
	if (!IsVisible()
		&& !IsHovered())
	{
		// Widget is already closed
		return;
	}

	SetVisibility(ESlateVisibility::Collapsed);

	SaveSettings();

	OnToggleSettings(false);

	OnCloseSettings();
}

// Flip-flop opens and closes the Settings menu
void USettingsWidget::ToggleSettings()
{
	if (IsVisible())
	{
		CloseSettings();
	}
	else
	{
		OpenSettings();
	}
}

// Add setting on UI.
void USettingsWidget::AddSetting(FSettingsPicker& Setting)
{
	const FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	FSettingsPrimary& PrimaryData = Setting.PrimaryData;
	TryBindStaticContext(PrimaryData);

	if (Setting.PrimaryData.bStartOnNextColumn)
	{
		StartNextColumn();
	}

	if (ChosenData == &Setting.Button)
	{
		AddSettingButton(PrimaryData, Setting.Button);
	}
	else if (ChosenData == &Setting.Checkbox)
	{
		AddSettingCheckbox(PrimaryData, Setting.Checkbox);
	}
	else if (ChosenData == &Setting.Combobox)
	{
		AddSettingCombobox(PrimaryData, Setting.Combobox);
	}
	else if (ChosenData == &Setting.Slider)
	{
		AddSettingSlider(PrimaryData, Setting.Slider);
	}
	else if (ChosenData == &Setting.TextLine)
	{
		AddSettingTextLine(PrimaryData, Setting.TextLine);
	}
	else if (ChosenData == &Setting.UserInput)
	{
		AddSettingUserInput(PrimaryData, Setting.UserInput);
	}
	else if (ChosenData == &Setting.CustomWidget)
	{
		AddSettingCustomWidget(PrimaryData, Setting.CustomWidget);
	}

	UpdateSettings(FGameplayTagContainer(PrimaryData.Tag));
}

// Add button on UI
void USettingsWidget::AddSettingButton(FSettingsPrimary& Primary, FSettingsButton& Data)
{
	const TSubclassOf<USettingButton> ButtonClass = USettingsDataAsset::Get().GetButtonClass();
	CreateSettingSubWidget(Primary, ButtonClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnButtonPressed.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddButton(Primary, Data);
}

// Add checkbox on UI
void USettingsWidget::AddSettingCheckbox(FSettingsPrimary& Primary, FSettingsCheckbox& Data)
{
	const TSubclassOf<USettingCheckbox> CheckboxClass = USettingsDataAsset::Get().GetCheckboxClass();
	CreateSettingSubWidget(Primary, CheckboxClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterBool.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterBool.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddCheckbox(Primary, Data);
}

// Add combobox on UI
void USettingsWidget::AddSettingCombobox(FSettingsPrimary& Primary, FSettingsCombobox& Data)
{
	const TSubclassOf<USettingCombobox> ComboboxClass = USettingsDataAsset::Get().GetComboboxClass();
	CreateSettingSubWidget(Primary, ComboboxClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetMembersFunctionName = Data.GetMembers.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetMembersFunctionName))
		{
			Data.OnGetMembers.BindUFunction(StaticContextObject, GetMembersFunctionName);
			Data.OnGetMembers.ExecuteIfBound(Data.Members);
		}

		const FName SetMembersFunctionName = Data.SetMembers.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetMembersFunctionName))
		{
			Data.OnSetMembers.BindUFunction(StaticContextObject, SetMembersFunctionName);
			Data.OnSetMembers.ExecuteIfBound(Data.Members);
		}

		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterInt.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterInt.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddCombobox(Primary, Data);
}

// Add slider on UI
void USettingsWidget::AddSettingSlider(FSettingsPrimary& Primary, FSettingsSlider& Data)
{
	const TSubclassOf<USettingSlider>& SliderClass = USettingsDataAsset::Get().GetSliderClass();
	CreateSettingSubWidget(Primary, SliderClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterFloat.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterFloat.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddSlider(Primary, Data);
}

// Add simple text on UI
void USettingsWidget::AddSettingTextLine(FSettingsPrimary& Primary, FSettingsTextLine& Data)
{
	const TSubclassOf<USettingTextLine>& TextLineClass = USettingsDataAsset::Get().GetTextLineClass();
	CreateSettingSubWidget(Primary, TextLineClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterText.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterText.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddTextLine(Primary, Data);
}

// Add text input on UI
void USettingsWidget::AddSettingUserInput(FSettingsPrimary& Primary, FSettingsUserInput& Data)
{
	const TSubclassOf<USettingUserInput>& UserInputClass = USettingsDataAsset::Get().GetUserInputClass();
	CreateSettingSubWidget(Primary, UserInputClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterName.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterName.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddUserInput(Primary, Data);
}

// Add custom widget on UI
void USettingsWidget::AddSettingCustomWidget(FSettingsPrimary& Primary, FSettingsCustomWidget& Data)
{
	CreateSettingSubWidget(Primary, Data.CustomWidgetClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterWidget.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterWidget.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddCustomWidget(Primary, Data);
}
