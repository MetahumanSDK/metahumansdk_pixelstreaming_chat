#include "MetahumanSDKMappingsAsset.h"
#include "Animation/Skeleton.h"
#include "Animation/SmartName.h"
#include "ReferenceSkeleton.h"

#if WITH_EDITOR
void UMetahumanSDKMappingsAsset::FillCurveNames()
{
    USkeleton* Skeleton = LoadObject<USkeleton>(nullptr, TEXT("/MetahumanSDK/Models/FaceExample_Skeleton"), nullptr);
    if (IsValid(Skeleton))
    {
        const FSmartNameMapping* SmartNameMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
        if (SmartNameMapping != nullptr)
        {
            TArray<FName> CurveNames;
            SmartNameMapping->FillNameArray(CurveNames);

            for (const FName& CurveName : CurveNames)
            {
                BlendShapeMappings.Add(CurveName.ToString(), TEXT(""));
            }
        }
    }
}
#endif
