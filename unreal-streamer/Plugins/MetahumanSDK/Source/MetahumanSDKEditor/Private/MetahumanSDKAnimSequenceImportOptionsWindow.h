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
#include "MetahumanSDKMappingsAsset.h"
#include "MetahumanSDKAPIInput.h"


#define LOCTEXT_NAMESPACE "MetahumanSDKAnimSequenceImportOptionsWindow"


class SMetahumanSDKAnimSequenceImportOptionsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMetahumanSDKAnimSequenceImportOptionsWindow)
		: _ImportOptions(nullptr)
	{}

	SLATE_ARGUMENT(UMetahumanSDKAnimSequenceImportOptions*, ImportOptions)
		SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs)
	{
		ImportOptions = InArgs._ImportOptions;
		Window = InArgs._WidgetWindow;
		bShouldImport = false;

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
					.Text(LOCTEXT("MetahumanSDKAnimSequenceImportOptionsWindow_Import", "Import"))
					.OnClicked(this, &SMetahumanSDKAnimSequenceImportOptionsWindow::OnImport)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("MetahumanSDKAnimSequenceImportOptionsWindow_Cancel", "Cancel"))
					.ToolTipText(LOCTEXT("MetahumanSDKAnimSequenceImportOptionsWindow_Cancel_ToolTip", "Cancels importing this Animation json file"))
					.OnClicked(this, &SMetahumanSDKAnimSequenceImportOptionsWindow::OnCancel)
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
		DetailsView->SetObject(ImportOptions);

		DetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateStatic([] { return true; }));
		DetailsView->OnFinishedChangingProperties().Add(FOnFinishedChangingProperties::FDelegate::CreateSP(this, &SMetahumanSDKAnimSequenceImportOptionsWindow::HandlePropertyChanged));

		DetailsViewBox->SetContent(DetailsView.ToSharedRef());
		
	}

	void HandlePropertyChanged(const FPropertyChangedEvent& InPropertyChangedEvent)
	{

	}

	virtual bool SupportsKeyboardFocus() const override { return true; }

    bool ShouldImport() const
    {
        return bShouldImport;
    }

	USkeleton* GetSkeleton() const 
	{
		return ImportOptions->Skeleton.Get();
	}

    ERichCurveInterpMode GetBlendShapeCurveInterpolationMode() const
    {
        return ImportOptions->BlendShapeCurveInterpolationMode;
    }

    FATLMappingsInfo GetMappingsInfo() const
    {
		if (GetMappingsMode() == EMetahumanSDKMappingsMode::ECustom)
		{
			if (GetCustomMappingsMode() == EMetahumanSDKCustomMappingsMode::EMappingAsset)
			{
				return FATLMappingsInfo(GetMappingsAsset(), GetBoneMappingAsset(), false);
			}
			else
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
		return ImportOptions->CustomMappingsMode;
	}
	EMetahumanSDKMappingsMode GetMappingsMode() const
	{
		return ImportOptions->MappingsMode;
	}

	UMetahumanSDKMappingsAsset* GetMappingsAsset() const
	{
		return ImportOptions->MappingsAsset.Get();
	}

	UMetahumanSDKBoneMappingAsset* GetBoneMappingAsset() const
	{
		return ImportOptions->BoneMappingAsset.Get();
	}

    UPoseAsset* GetPoseAsset() const
    {
        return ImportOptions->PoseAsset.Get();
    }

protected:
	FReply OnImport()
	{
		if (!ImportOptions->Skeleton.IsValid())
		{
			FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::No, INVTEXT("Provide skeleton to proceed!"));
			return FReply::Handled();
		}

		bShouldImport = true;
		if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	FReply OnCancel()
	{
		bShouldImport = false;
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
	UMetahumanSDKAnimSequenceImportOptions* ImportOptions;
	TWeakPtr<SWindow> Window;
	bool bShouldImport;

	FText PickedImportMode;
};


#undef LOCTEXT_NAMESPACE