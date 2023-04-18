#pragma once

#include "CoreMinimal.h"
#include "Curves/RealCurve.h"
#include "MetahumanSDKAPIInput.h"


class USkeleton;
class UAnimSequence;
class UMetahumanSDKMappingsAsset;
class UPoseAsset;
class FFeedbackContext;
struct FATLResponse;


struct FPoseInfo
{
    FName PoseName;
    int32 ATLResponse_BlendShapeIndex = INDEX_NONE;
    TArray<FName> AffectedCurveNames;
    TArray<float> AffectedCurveWeights;
};


class METAHUMANSDK_API FAnimSequenceFactory
{
    FAnimSequenceFactory();
    FAnimSequenceFactory(FVTableHelper& Helper);
    virtual ~FAnimSequenceFactory();
public:
    static UAnimSequence* CreateAnimSequence(USkeleton* Skeleton, ERichCurveInterpMode CurveInterpMode, const FATLMappingsInfo& MappingsInfo, FATLResponse* ATLOutput, FString& Error, UObject* InParent = nullptr, const FString& AnimSequenceName = TEXT(""));

private:
    static void ConvertMappingsAssetToPoseInfos(FATLResponse* ATLResponse, const FATLMappingsInfo& Mappings, TArray<FPoseInfo>& OutPoseInfos);

    static UAnimSequence* CreateAnimSequence(USkeleton* Skeleton, ERichCurveInterpMode CurveInterpMode, FATLResponse* ATLOutput, const TArray<FPoseInfo>& PoseInfos, const TMap<FString, FString>& BoneNamesMapping, FString& Error, UObject* InParent = nullptr, const FString& AnimSequenceName = TEXT(""), bool bSetUpForMetahumans = false);
};