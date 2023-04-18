#include "AudioToLipsyncExtension.h"

#include "AssetToolsModule.h"
#include "UObject/Object.h"
#include "ToolMenus.h"
#include "ContentBrowserMenuContexts.h"
#include "Sound/SoundWave.h"
#include "MetahumanSDKAnimSequenceAssetFactory.h"

#define LOCTEXT_NAMESPACE "AudioToLipsyncExtension"

void FAudioToLipsyncExtension::RegisterMenus()
{
	if (!UToolMenus::IsToolMenuUIEnabled())
	{
		return;
	}

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.SoundWave");
	FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");

	Section.AddDynamicEntry("SoundWaveAssetConversion_CreateLipsyncAnimation", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
		const TAttribute<FText> Label = LOCTEXT("SoundWave_CreateLipsyncAnimation", "Create Lipsync Animation");
		const TAttribute<FText> ToolTip = LOCTEXT("SoundWave_CreateLipsyncAnimationTooltip", "Creates an lipsync animation asset using the selected sound wave.");
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimSequence");
#else
		const FSlateIcon Icon = FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.AnimSequence");	
#endif
		const FToolMenuExecuteAction UIAction = FToolMenuExecuteAction::CreateStatic(&FAudioToLipsyncExtension::ExecuteCreateLipsyncAnimation);

		InSection.AddMenuEntry("SoundWave_CreateLipsyncAnimation", Label, ToolTip, Icon, UIAction);
	}));
}

void FAudioToLipsyncExtension::ExecuteCreateLipsyncAnimation(const FToolMenuContext& MenuContext)
{
	UContentBrowserAssetContextMenuContext* Context = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
	if (!Context)
	{
		return;
	}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	if (Context->SelectedAssets.Num() == 0)
	{
		return;
	}
#else
	if (Context->SelectedObjects.Num() == 0)
	{
		return;
	}
#endif
	const FString DefaultSuffix = TEXT("_Lipsync");
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// Create the factory used to generate the asset
	UMetahumanSDKAnimSequenceAssetFactory* Factory = NewObject<UMetahumanSDKAnimSequenceAssetFactory>();
	
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	// only converts 0th selected object for now (see return statement)
	for (auto &AssetData : Context->SelectedAssets)
	{
		// stage the soundwave on the factory to be used during asset creation
		USoundWave* Wave = Cast<USoundWave>(AssetData.GetAsset());
#else
	// only converts 0th selected object for now (see return statement)
	for (const TWeakObjectPtr<UObject>& Object : Context->SelectedObjects)
	{
		// stage the soundwave on the factory to be used during asset creation
		USoundWave* Wave = Cast<USoundWave>(Object);
#endif

		check(Wave);
		Factory->SourceSoundWave = Wave; // WeakPtr gets reset by the Factory after it is consumed

		// Determine an appropriate name
		FString Name;
		FString PackagePath;
		AssetToolsModule.Get().CreateUniqueAssetName(Wave->GetOutermost()->GetName(), DefaultSuffix, PackagePath, Name);
		
		// create new asset
		AssetToolsModule.Get().CreateAsset(Name, FPackageName::GetLongPackagePath(PackagePath), UAnimSequence::StaticClass(), Factory);
	}
}

#undef LOCTEXT_NAMESPACE