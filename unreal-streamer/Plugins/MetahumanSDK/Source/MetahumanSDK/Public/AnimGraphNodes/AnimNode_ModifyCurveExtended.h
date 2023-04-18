#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_ModifyCurveExtended.generated.h"


/** Easy way to modify curve values on a pose by mask */
USTRUCT(BlueprintInternalUseOnly)
struct METAHUMANSDK_API FAnimNode_ModifyCurveExtended : public FAnimNode_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
    FPoseLink SourcePose;

    UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
    FPoseLink CurveOverridePose;

    /* Curve name prefix to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Params)
    FString CurveMask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Params, meta = (PinShownByDefault))
    float Alpha;

    FAnimNode_ModifyCurveExtended();

    // FAnimNode_Base interface
    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
    virtual void Evaluate_AnyThread(FPoseContext& Output) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
    // End of FAnimNode_Base interface
};
