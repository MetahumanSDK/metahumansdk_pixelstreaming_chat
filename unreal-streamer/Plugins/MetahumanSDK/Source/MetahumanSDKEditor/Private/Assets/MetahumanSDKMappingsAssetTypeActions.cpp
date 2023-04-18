#include "MetahumanSDKMappingsAssetTypeActions.h"
#include "MetahumanSDKMappingsAsset.h"
#include "ToolMenuSection.h"

FMetahumanSDKMappingsAssetTypeActions::FMetahumanSDKMappingsAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
    : MyAssetCategory(InAssetCategory)
{
    
}

FText FMetahumanSDKMappingsAssetTypeActions::GetName() const
{
    return INVTEXT("MetahumanSDKMappings");
}

FColor FMetahumanSDKMappingsAssetTypeActions::GetTypeColor() const
{
    return FColor::Cyan;
}

UClass* FMetahumanSDKMappingsAssetTypeActions::GetSupportedClass() const
{
    return UMetahumanSDKMappingsAsset::StaticClass();
}

void FMetahumanSDKMappingsAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
    FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
}

uint32 FMetahumanSDKMappingsAssetTypeActions::GetCategories()
{
    return MyAssetCategory;
}


