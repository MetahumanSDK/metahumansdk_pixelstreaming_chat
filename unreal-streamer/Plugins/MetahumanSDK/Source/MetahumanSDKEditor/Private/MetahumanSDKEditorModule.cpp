#include "MetahumanSDKEditorModule.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "ISettingsModule.h"
#include "AssetTypeCategories.h"
#include "ToolMenus.h"
#include "AudioToLipsyncExtension.h"
#include "MetahumanSDKSettings.h"
#include "MetahumanSDKMappingsAssetTypeActions.h"
#include "MetahumanSDKBoneMappingAssetTypeActions.h"
#include "MetahumanSDKSettingsDetails.h"
#include "TextToSpeechExtension.h"

#define LOCTEXT_NAMESPACE "FMetahumanSDKEditorModule"

void FMetahumanSDKEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    RegisterSettings();

    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    EAssetTypeCategories::Type MetahumanSDKAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("MetahumanSDK")), LOCTEXT("MetahumanSDKCategory", "MetahumanSDK"));

    TSharedPtr<IAssetTypeActions> Action = MakeShareable(new FMetahumanSDKMappingsAssetTypeActions(MetahumanSDKAssetCategoryBit));
    CurrentActions.Add(Action);

    TSharedPtr<IAssetTypeActions> BoneMappingAction = MakeShareable(new FMetahumanSDKBoneMappingAssetTypeActions(MetahumanSDKAssetCategoryBit));
    CurrentActions.Add(BoneMappingAction);

    AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
    AssetTools.RegisterAssetTypeActions(BoneMappingAction.ToSharedRef());

    // AssetTools.RegisterAssetTypeActions(MakeShared<FAssetTypeActions_AudioToLipsyncAssetTypeActions>());

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMetahumanSDKEditorModule::RegisterMenus));

    FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(UMetahumanSDKSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FMetahumanSDKSettingsDetails::MakeInstance));

}

void FMetahumanSDKEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
    UnregisterSettings();
}

void FMetahumanSDKEditorModule::RegisterMenus()
{
    FAudioToLipsyncExtension::RegisterMenus();
    FTextToSpeechExtension::RegisterMenus();
}

void FMetahumanSDKEditorModule::RegisterSettings()
{
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

    if (SettingsModule != nullptr)
    {
        SettingsModule->RegisterSettings(
            "Project",
            "Plugins",
            "MetahumanSDK",
            LOCTEXT("MetahumanSDKSettingsName", "Metahuman SDK"),
            LOCTEXT("MetahumanSDKSettingsDescription", "Configure Metahuman SDK plug-in"),
            GetMutableDefault<UMetahumanSDKSettings>()
        );
    }
}

void FMetahumanSDKEditorModule::UnregisterSettings()
{
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

    if (SettingsModule != nullptr)
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "MetahumanSDK");
    }
}

IMPLEMENT_MODULE(FMetahumanSDKEditorModule, MetahumanSDKEditor);

#undef LOCTEXT_NAMESPACE