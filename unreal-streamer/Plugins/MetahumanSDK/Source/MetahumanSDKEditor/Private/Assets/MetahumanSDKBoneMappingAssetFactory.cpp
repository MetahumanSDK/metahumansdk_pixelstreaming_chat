#include "MetahumanSDKBoneMappingAssetFactory.h"
#include "MetahumanSDKBoneMappingAsset.h"

UMetahumanSDKBoneMappingAssetFactory::UMetahumanSDKBoneMappingAssetFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bCreateNew = true;
    bEditAfterNew = true;
    SupportedClass = UMetahumanSDKBoneMappingAsset::StaticClass();
}

bool UMetahumanSDKBoneMappingAssetFactory::ConfigureProperties()
{
    return true;
}

UObject* UMetahumanSDKBoneMappingAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    UMetahumanSDKBoneMappingAsset* MappingAsset = NewObject<UMetahumanSDKBoneMappingAsset>(InParent, Class, Name, Flags | RF_Transactional);
    

    return MappingAsset;
}
