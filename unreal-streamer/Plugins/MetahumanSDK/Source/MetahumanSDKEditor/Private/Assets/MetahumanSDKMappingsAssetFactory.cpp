#include "MetahumanSDKMappingsAssetFactory.h"
#include "MetahumanSDKMappingsAsset.h"

UMetahumanSDKMappingsAssetFactory::UMetahumanSDKMappingsAssetFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bCreateNew = true;
    bEditAfterNew = true;
    SupportedClass = UMetahumanSDKMappingsAsset::StaticClass();
}

bool UMetahumanSDKMappingsAssetFactory::ConfigureProperties()
{
    return true;
}

UObject* UMetahumanSDKMappingsAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    UMetahumanSDKMappingsAsset* MappingsAsset = NewObject<UMetahumanSDKMappingsAsset>(InParent, Class, Name, Flags | RF_Transactional);
    MappingsAsset->FillCurveNames();

    return MappingsAsset;
}
