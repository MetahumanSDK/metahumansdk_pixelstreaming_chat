#include "AnimNode_ModifyCurveExtended.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"

FAnimNode_ModifyCurveExtended::FAnimNode_ModifyCurveExtended()
{
    Alpha = 1.f;
}

void FAnimNode_ModifyCurveExtended::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
    Super::Initialize_AnyThread(Context);
    SourcePose.Initialize(Context);
    CurveOverridePose.Initialize(Context);

}

void FAnimNode_ModifyCurveExtended::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
    Super::CacheBones_AnyThread(Context);
    SourcePose.CacheBones(Context);
    CurveOverridePose.CacheBones(Context);
}

void FAnimNode_ModifyCurveExtended::Evaluate_AnyThread(FPoseContext& Output)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
    FPoseContext SourceData(Output);
    SourcePose.Evaluate(SourceData);
    
    FPoseContext CurveOverrideData(Output);
    CurveOverridePose.Evaluate(CurveOverrideData);

    Output = SourceData;

    //    Morph target and Material parameter curves
    USkeleton* Skeleton = Output.AnimInstanceProxy->GetSkeleton();

    const FSmartNameMapping* SmartNameContainer = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
    checkf(SmartNameContainer != nullptr, TEXT("Skeleton should always have smart name container"));
    
    TArray<FName> CurveNames;
    SmartNameContainer->FillNameArray(CurveNames);

    const float UseAlpha = FMath::Clamp(Alpha, 0.f, 1.f);

    for (int32 ModIdx = 0; ModIdx < CurveNames.Num(); ModIdx++)
    {
        FName CurveName = CurveNames[ModIdx];
        SmartName::UID_Type NameUID = Skeleton->GetUIDByName(USkeleton::AnimCurveMappingName, CurveName);
        if (NameUID != SmartName::MaxUID && CurveName.ToString().StartsWith(CurveMask))
        {
            const float CurveOverrideValue = CurveOverrideData.Curve.Get(NameUID);
            const float CurveCurrentValue = Output.Curve.Get(NameUID);
            const float CurveFinalValue = FMath::Lerp(CurveCurrentValue, CurveOverrideValue, UseAlpha);

            Output.Curve.Set(NameUID, CurveFinalValue);
        }
    }
}

void FAnimNode_ModifyCurveExtended::Update_AnyThread(const FAnimationUpdateContext& Context)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)
    // Run update on input pose nodes
    SourcePose.Update(Context);
    CurveOverridePose.Update(Context);

    // Evaluate any BP logic plugged into this node
    GetEvaluateGraphExposedInputs().Execute(Context);
}
