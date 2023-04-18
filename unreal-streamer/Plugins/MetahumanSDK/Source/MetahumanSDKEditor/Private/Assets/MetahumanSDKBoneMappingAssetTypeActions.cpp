#include "MetahumanSDKBoneMappingAssetTypeActions.h"
#include "MetahumanSDKBoneMappingAsset.h"
#include "ToolMenuSection.h"

FMetahumanSDKBoneMappingAssetTypeActions::FMetahumanSDKBoneMappingAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
    : MyAssetCategory(InAssetCategory)
{
    
}

FText FMetahumanSDKBoneMappingAssetTypeActions::GetName() const
{
    return INVTEXT("MetahumanSDKBoneMapping");
}

FColor FMetahumanSDKBoneMappingAssetTypeActions::GetTypeColor() const
{
    return FColor::Cyan;
}

UClass* FMetahumanSDKBoneMappingAssetTypeActions::GetSupportedClass() const
{
    return UMetahumanSDKBoneMappingAsset::StaticClass();
}

void FMetahumanSDKBoneMappingAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
    FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
}

uint32 FMetahumanSDKBoneMappingAssetTypeActions::GetCategories()
{
    return MyAssetCategory;
}


