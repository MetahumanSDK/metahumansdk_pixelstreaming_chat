#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MetahumanSDKMappingsAsset.generated.h"

/*
*/

UCLASS(BlueprintType, Blueprintable)
class METAHUMANSDK_API UMetahumanSDKMappingsAsset : public UDataAsset
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    void FillCurveNames();
#endif

    /** Blendshape mappings */
    UPROPERTY(EditAnywhere, Category = "MetahumanSDKMappingsAsset")
    TMap<FString, FString> BlendShapeMappings;    
};
