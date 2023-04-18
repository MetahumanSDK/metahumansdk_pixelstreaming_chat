#pragma once

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "IDetailsView.h"
#include "EditorStyleSet.h"
#include "Animation/Skeleton.h"
#include "Animation/PoseAsset.h"
#include "MetahumanSDKSoundWaveGenerateOptions.h"
#include "MetahumanSDKMappingsAsset.h"
#include "MetahumanSDKAPIInput.h"


#define LOCTEXT_NAMESPACE "MetahumanSDKSoundWaveGenerateOptionsWindow"


class SMetahumanSDKSoundWaveGenerateOptionsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMetahumanSDKSoundWaveGenerateOptionsWindow)
		: _GenerateOptions(nullptr)
	{}

	SLATE_ARGUMENT(UMetahumanSDKSoundWaveGenerateOptions*, GenerateOptions)
		SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs)
	{
		GenerateOptions = InArgs._GenerateOptions;
		Window = InArgs._WidgetWindow;
		bShouldGenerate = false;

		TSharedPtr<SBox> DetailsViewBox;
		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SAssignNew(DetailsViewBox, SBox)
				.MaxDesiredHeight(450.0f)
				.MinDesiredWidth(550.0f)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(2)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(2)
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("MetahumanSDKSoundWaveGenerateOptionsWindow_Generate", "Generate"))
					.OnClicked(this, &SMetahumanSDKSoundWaveGenerateOptionsWindow::OnGenerate)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("MetahumanSDKSoundWaveGenerateOptionsWindow_Cancel", "Cancel"))
					.ToolTipText(LOCTEXT("MetahumanSDKSoundWaveGenerateOptionsWindow_Cancel_ToolTip", "Cancels Generating this lipsync"))
					.OnClicked(this, &SMetahumanSDKSoundWaveGenerateOptionsWindow::OnCancel)
				]
			]
		];

		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		FDetailsViewArgs DetailsViewArgs;
		{
            DetailsViewArgs.bAllowSearch = false;
            DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		}
		
		TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		DetailsView->SetObject(GenerateOptions);

		DetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateStatic([] { return true; }));
		DetailsView->OnFinishedChangingProperties().Add(FOnFinishedChangingProperties::FDelegate::CreateSP(this, &SMetahumanSDKSoundWaveGenerateOptionsWindow::HandlePropertyChanged));

		DetailsViewBox->SetContent(DetailsView.ToSharedRef());
		
	}

	void HandlePropertyChanged(const FPropertyChangedEvent& InPropertyChangedEvent)
	{

	}

	virtual bool SupportsKeyboardFocus() const override { return true; }

    bool ShouldGenerate() const
    {
        return bShouldGenerate;
    }

	FString GetEngine() const 
	{
		return StaticEnum<ETTSEngine>()->GetDisplayValueAsText(GenerateOptions->TTSEngine).ToString();
	}

	FString GetVoice() const 
	{
		switch (GenerateOptions->TTSEngine)
		{
			case ETTSEngine::TTSEngineGoogle:
				return StaticEnum<ETTSVoiceGoogle>()->GetDisplayValueAsText(GenerateOptions->TTSVoiceGoogle).ToString();
			case ETTSEngine::TTSEngineAzure:
				return StaticEnum<ETTSVoiceAzure>()->GetDisplayValueAsText(GenerateOptions->TTSVoiceAzure).ToString();
			default:
				return TEXT("");
		}
	}

	FString GetText() const 
	{
		return GenerateOptions->Text;
	}

protected:
	FReply OnGenerate()
	{
		bShouldGenerate = true;
		if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	FReply OnCancel()
	{
		bShouldGenerate = false;
		if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			return OnCancel();
		}

		return FReply::Unhandled();
	}

private:
	UMetahumanSDKSoundWaveGenerateOptions* GenerateOptions;
	TWeakPtr<SWindow> Window;
	bool bShouldGenerate;

	FText PickedGenerateMode;
};


#undef LOCTEXT_NAMESPACE