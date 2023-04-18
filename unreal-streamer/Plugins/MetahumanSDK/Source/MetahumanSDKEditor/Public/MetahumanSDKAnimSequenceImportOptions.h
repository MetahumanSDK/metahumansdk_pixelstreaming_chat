#pragma once

#include "CoreMinimal.h"
#include "MetahumanSDKAPIInput.h"
#include "Curves/RichCurve.h"
#include "MetahumanSDKAnimSequenceImportOptions.generated.h"


UENUM()
enum class EMetahumanSDKMappingsMode : uint8
{
    ENone = 0,
    ECustom = 1,
    EMetahuman = 2,
};

UENUM()
enum class EMetahumanSDKCustomMappingsMode : uint8
{
    EMappingAsset = 0,
    EPoseAsset = 1,
};

class UMetahumanSDKBoneMappingAsset;
class UMetahumanSDKMappingsAsset;
class USkeleton;
class UPoseAsset;


UCLASS()
class METAHUMANSDKEDITOR_API UMetahumanSDKAnimSequenceImportOptions : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = ImportOptions)
    TEnumAsByte<ERichCurveInterpMode> BlendShapeCurveInterpolationMode = ERichCurveInterpMode::RCIM_Cubic;

    UPROPERTY(EditAnywhere, Category = ImportOptions)
    TSoftObjectPtr<USkeleton> Skeleton = nullptr;
    
    UPROPERTY(EditAnywhere, Category = ImportOptions)
    EMetahumanSDKMappingsMode MappingsMode;

    UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditConditionHides, HideEditConditionToggle, EditCondition = "MappingsMode == EMetahumanSDKMappingsMode::ECustom && bCanUsePoseAsset"))
    EMetahumanSDKCustomMappingsMode CustomMappingsMode;

    UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditConditionHides, HideEditConditionToggle, EditCondition = "MappingsMode == EMetahumanSDKMappingsMode::ECustom && CustomMappingsMode == EMetahumanSDKCustomMappingsMode::EPoseAsset && bCanUsePoseAsset"))
    TSoftObjectPtr<UPoseAsset> PoseAsset = nullptr;

    UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditConditionHides, HideEditConditionToggle, EditCondition = "MappingsMode == EMetahumanSDKMappingsMode::ECustom && CustomMappingsMode == EMetahumanSDKCustomMappingsMode::EMappingAsset"))
    TSoftObjectPtr<UMetahumanSDKMappingsAsset> MappingsAsset = nullptr;

    UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditConditionHides, HideEditConditionToggle, EditCondition = "MappingsMode == EMetahumanSDKMappingsMode::ECustom"))
    TSoftObjectPtr<UMetahumanSDKBoneMappingAsset> BoneMappingAsset = nullptr;

protected:
    void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);

        if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMetahumanSDKAnimSequenceImportOptions, MappingsMode))
        {
            if (MappingsMode == EMetahumanSDKMappingsMode::EMetahuman)
            {
                LoadMetahumanPoseAsset();
                PoseAsset = MetahumanPoseAsset;
            }
        }
    }

    UPROPERTY()
    bool bCanUsePoseAsset = (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 25) || ENGINE_MAJOR_VERSION == 5;

    void LoadMetahumanPoseAsset()
    {
        if (!MetahumanPoseAsset && bCanUsePoseAsset)
        {
            MetahumanPoseAsset = LoadObject<UPoseAsset>(nullptr, TEXT("/MetahumanSDK/Data/mh_dhs_mapping_anim_PoseAsset"));
        }
    }

    UPROPERTY()
    TSoftObjectPtr<UPoseAsset> MetahumanPoseAsset = nullptr;

};