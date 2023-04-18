#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MetahumanSDKBoneMappingAsset.generated.h"

/*
*/

UCLASS(BlueprintType, Blueprintable)
class METAHUMANSDK_API UMetahumanSDKBoneMappingAsset : public UDataAsset
{
    GENERATED_BODY()

public:
 

   /** Bone mappings */
    UPROPERTY(EditAnywhere, Category = "MetahumanSDKMappingsAsset")
    TMap<FString, FString> BoneMappings;  
};
