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
#include "MetahumanSDKAnimSequenceImportOptions.h"
#include "MetahumanSDKAnimSequenceGenerateOptions.h"
#include "MetahumanSDKMappingsAsset.h"
#include "MetahumanSDKAPIInput.h"


#define LOCTEXT_NAMESPACE "MetahumanSDKAnimSequenceGenerateOptionsWindow"


class SMetahumanSDKAnimSequenceGenerateOptionsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMetahumanSDKAnimSequenceGenerateOptionsWindow)
		: _GenerateOptions(nullptr)
	{}

	SLATE_ARGUMENT(UMetahumanSDKAnimSequenceGenerateOptions*, GenerateOptions)
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
				SNew(SBorder)
				.Padding(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(FMargin(15, 10, 10, 10))
					.FillWidth(1.0f)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
#if ( ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1 )
						.Font(FAppStyle::GetFontStyle( TEXT("PropertyWindow.NormalFont")))
#else // UE < 5.1
						.Font(FEditorStyle::GetFontStyle( TEXT("PropertyWindow.NormalFont")))
#endif						
						.Text(FText::FromString(FString(TEXT("Recommended sound specification: PCM WAV, Mono, 16 kHz, 16 bps"))))
					]
				]
			]
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
					.Text(LOCTEXT("MetahumanSDKAnimSequenceGenerateOptionsWindow_Generate", "Generate"))
					.OnClicked(this, &SMetahumanSDKAnimSequenceGenerateOptionsWindow::OnGenerate)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("MetahumanSDKAnimSequenceGenerateOptionsWindow_Cancel", "Cancel"))
					.ToolTipText(LOCTEXT("MetahumanSDKAnimSequenceGenerateOptionsWindow_Cancel_ToolTip", "Cancels Generating this lipsync"))
					.OnClicked(this, &SMetahumanSDKAnimSequenceGenerateOptionsWindow::OnCancel)
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
		DetailsView->OnFinishedChangingProperties().Add(FOnFinishedChangingProperties::FDelegate::CreateSP(this, &SMetahumanSDKAnimSequenceGenerateOptionsWindow::HandlePropertyChanged));

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

	USoundWave* GetSound() const
	{
		return GenerateOptions->Sound.Get();
	}
    
	EExplicitEmotion GetExplicitEmotion() const
	{
		return GenerateOptions->ExplicitEmotion;
	}
	
	bool GetGenerateEyeMovement() const
	{
		return GenerateOptions->bGenerateEyeMovement;
	}
	
	bool GetGenerateNeckMovement() const
	{
		return GenerateOptions->bGenerateNeckMovement;
	}
	
	USkeleton* GetSkeleton() const 
	{
		return GenerateOptions->Skeleton.Get();
	}

    ERichCurveInterpMode GetBlendShapeCurveInterpolationMode() const
    {
        return GenerateOptions->BlendShapeCurveInterpolationMode;
    }

    FATLMappingsInfo GetMappingsInfo() const
    {
        if (GetMappingsMode() == EMetahumanSDKMappingsMode::ECustom)
        {
			if(GetCustomMappingsMode() == EMetahumanSDKCustomMappingsMode::EMappingAsset)
			{
				return FATLMappingsInfo(GetMappingsAsset(), GetBoneMappingAsset(), false);
			} else
			{
				return FATLMappingsInfo(GetPoseAsset(), GetBoneMappingAsset(), false);
			}
        }
        else if (GetMappingsMode() == EMetahumanSDKMappingsMode::EMetahuman)
        {
            return FATLMappingsInfo(GetPoseAsset(), GetBoneMappingAsset(), true);
        }
        return FATLMappingsInfo();
    }

protected:
	EMetahumanSDKCustomMappingsMode GetCustomMappingsMode() const
	{
		return GenerateOptions->CustomMappingsMode;
	}
	EMetahumanSDKMappingsMode GetMappingsMode() const
	{
		return GenerateOptions->MappingsMode;
	}

	UMetahumanSDKMappingsAsset* GetMappingsAsset() const
	{
		return GenerateOptions->MappingsAsset.Get();
	}

	UMetahumanSDKBoneMappingAsset* GetBoneMappingAsset() const
	{
		return GenerateOptions->BoneMappingAsset.Get();
	}

    UPoseAsset* GetPoseAsset() const
    {
        return GenerateOptions->PoseAsset.Get();
    }

protected:
	FReply OnGenerate()
	{
		if (!GenerateOptions->Sound.IsValid())
		{
			FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::No, INVTEXT("Provide sound to proceed!"));
			return FReply::Handled();
		}
		
		if (!GenerateOptions->Skeleton.IsValid())
		{
			FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::No, INVTEXT("Provide skeleton to proceed!"));
			return FReply::Handled();
		}

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
	UMetahumanSDKAnimSequenceGenerateOptions* GenerateOptions;
	TWeakPtr<SWindow> Window;
	bool bShouldGenerate;

	FText PickedGenerateMode;
};


#undef LOCTEXT_NAMESPACE