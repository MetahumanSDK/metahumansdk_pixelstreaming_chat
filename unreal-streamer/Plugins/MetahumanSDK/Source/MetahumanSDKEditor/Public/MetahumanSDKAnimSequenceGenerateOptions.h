#pragma once

#include "CoreMinimal.h"
#include "MetahumanSDKAPIInput.h"
#include "MetahumanSDKAnimSequenceImportOptions.h"
#include "MetahumanSDKAnimSequenceGenerateOptions.generated.h"

UCLASS()
class METAHUMANSDKEDITOR_API UMetahumanSDKAnimSequenceGenerateOptions : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = GenerateOptions)
    TSoftObjectPtr<USoundWave> Sound;
    
    UPROPERTY(EditAnywhere, Category = GenerateOptions)
    EExplicitEmotion ExplicitEmotion = EExplicitEmotion::ECalm;
    
    UPROPERTY(EditAnywhere, Category = GenerateOptions)
    bool bGenerateEyeMovement = false;
    
    UPROPERTY(EditAnywhere, Category = GenerateOptions)
    bool bGenerateNeckMovement = false;

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
        
        if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMetahumanSDKAnimSequenceGenerateOptions, MappingsMode))
        {
            if(MappingsMode == EMetahumanSDKMappingsMode::EMetahuman)
            {
                LoadMetahumanPoseAsset();
                PoseAsset = MetahumanPoseAsset;
                LoadMetahumanBoneMappingAsset();
                BoneMappingAsset = MetahumanBoneMappingAsset;

            }
        }
    }

    UPROPERTY()
    bool bCanUsePoseAsset =  ( ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 25 ) || ENGINE_MAJOR_VERSION == 5;

    void LoadMetahumanPoseAsset()
    {
        if(!MetahumanPoseAsset && bCanUsePoseAsset)
        {
            MetahumanPoseAsset = LoadObject<UPoseAsset>(nullptr, TEXT("/MetahumanSDK/Data/mh_dhs_mapping_anim_PoseAsset"));
        }
    }

    void LoadMetahumanBoneMappingAsset()
    {
        if (!MetahumanBoneMappingAsset) {
            MetahumanBoneMappingAsset = LoadObject<UMetahumanSDKBoneMappingAsset>(nullptr, TEXT("/MetaHumanSDK/Data/Default_Metahuman_Bone_Mapping"));
        }
    }

    UPROPERTY()
    TSoftObjectPtr<UPoseAsset> MetahumanPoseAsset = nullptr;

    UPROPERTY()
    TSoftObjectPtr<UMetahumanSDKBoneMappingAsset> MetahumanBoneMappingAsset = nullptr;

};