#include "AssetFactories/AnimSequenceFactory.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimCurveTypes.h"
#include "Animation/AnimCompressionTypes.h"
#include "Animation/PoseAsset.h"
#include "Animation/Skeleton.h"
#include "Animation/SmartName.h"
#include "Animation/AnimBoneCompressionSettings.h"
#include "Animation/AnimCurveCompressionCodec_CompressedRichCurve.h"
#include "Animation/AnimCurveCompressionSettings.h"
#include "AnimationUtils.h"
#include "Curves/RichCurve.h"
#include "Serialization/MemoryWriter.h"

#if WITH_EDITOR
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#endif

#include "MetahumanSDKMappingsAsset.h"
#include "MetahumanSDKResponses.h"
#include "MetahumanSDKSettings.h"
#include "Animation/AnimData/AnimDataModel.h"
#include "Codecs/MetahumanSDKBoneCompressionCodec.h"
#include "Interfaces/ITargetPlatformManagerModule.h"

#define LOCTEXT_NAMESPACE "MetahumanSDKAnimSequenceFactory"
DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKAnimSequenceFactory, Log, All);


FAnimSequenceFactory::FAnimSequenceFactory() = default;

FAnimSequenceFactory::FAnimSequenceFactory(FVTableHelper& Helper)
{
    FAnimSequenceFactory();
}

FAnimSequenceFactory::~FAnimSequenceFactory() = default;


UAnimSequence* FAnimSequenceFactory::CreateAnimSequence(USkeleton* Skeleton, ERichCurveInterpMode CurveInterpMode, const FATLMappingsInfo& MappingsInfo, FATLResponse* ATLOutput, FString& Error, UObject* InParent /*= nullptr*/, const FString& AnimSequenceName/* = TEXT("")*/)
{
    QUICK_SCOPE_CYCLE_COUNTER(MetahumanSDK_CreateAnimSequence);

    if (!IsValid(Skeleton))
    {
        UE_LOG(LogMetahumanSDKAnimSequenceFactory, Warning, TEXT("Can't create AnimSequence cause no skeleton was provided"), Skeleton);
        return nullptr;
    }

    const double StartTime = FPlatformTime::Seconds();

    TArray<FPoseInfo> PoseInfos;
    ConvertMappingsAssetToPoseInfos(ATLOutput, MappingsInfo, PoseInfos);

    TMap<FString, FString> BonesMapping;
    if(IsValid(MappingsInfo.GetBoneMappingAsset()))
    {
        BonesMapping.Append(MappingsInfo.GetBoneMappingAsset()->BoneMappings);
    }
    
    UAnimSequence* Result = CreateAnimSequence(Skeleton, CurveInterpMode, ATLOutput, PoseInfos, BonesMapping, Error, InParent, AnimSequenceName, MappingsInfo.ShouldSetUpForMetahumans());

    const double EndTime = FPlatformTime::Seconds();
    UE_LOG(LogMetahumanSDKAnimSequenceFactory, Log, TEXT("Creating AnimSequence took %f seconds to complete"), EndTime - StartTime);

    return Result;
}

void CompressCurves(const FCompressibleAnimData& AnimSeq, TArray<uint8>& CompressedCurveBytes)
{
    // This mirrors UAnimCurveCompressionCodec_CompressedRichCurve settings
    float MaxCurveError = 0.0f;
    bool UseAnimSequenceSampleRate = true;
    float ErrorSampleRate = 60.0f;

    // This mirrors in part the FCompressedRichCurve
    struct FCurveDesc
    {
        TEnumAsByte<ERichCurveCompressionFormat> CompressionFormat;
        TEnumAsByte<ERichCurveKeyTimeCompressionFormat> KeyTimeCompressionFormat;
        TEnumAsByte<ERichCurveExtrapolation> PreInfinityExtrap;
        TEnumAsByte<ERichCurveExtrapolation> PostInfinityExtrap;
        FCompressedRichCurve::TConstantValueNumKeys ConstantValueNumKeys;
        int32 KeyDataOffset;
    };
    
#if ENGINE_MAJOR_VERSION == 5
    int32 NumCurves = AnimSeq.RawFloatCurves.Num();
#else
    int32 NumCurves = AnimSeq.RawCurveData.FloatCurves.Num();
#endif
    
    TArray<FCurveDesc> Curves;
    Curves.Reserve(NumCurves);
    Curves.AddUninitialized(NumCurves);

    int32 KeyDataOffset = 0;
    KeyDataOffset += sizeof(FCurveDesc) * NumCurves;

#if ENGINE_MAJOR_VERSION == 5
    const FAnimKeyHelper Helper(AnimSeq.SequenceLength, AnimSeq.NumberOfKeys);
#else
    const FAnimKeyHelper Helper(AnimSeq.SequenceLength, AnimSeq.NumFrames);
#endif
    const float SampleRate = UseAnimSequenceSampleRate ? Helper.KeysPerSecond() : ErrorSampleRate;

    TArray<uint8> KeyData;

    for (int32 CurveIndex = 0; CurveIndex < NumCurves; ++CurveIndex)
    {
#if ENGINE_MAJOR_VERSION == 5
        const FFloatCurve& Curve = AnimSeq.RawFloatCurves[CurveIndex];
#else
        const FFloatCurve& Curve = AnimSeq.RawCurveData.FloatCurves[CurveIndex];
#endif

        FRichCurve RawCurve = Curve.FloatCurve;    // Copy
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
        RawCurve.RemoveRedundantAutoTangentKeys(MaxCurveError);
#else
        RawCurve.RemoveRedundantKeys(MaxCurveError);
#endif
        
        

        FCompressedRichCurve CompressedCurve;
        RawCurve.CompressCurve(CompressedCurve, MaxCurveError, SampleRate);

        FCurveDesc& CurveDesc = Curves[CurveIndex];
        CurveDesc.CompressionFormat = CompressedCurve.CompressionFormat;
        CurveDesc.KeyTimeCompressionFormat = CompressedCurve.KeyTimeCompressionFormat;
        CurveDesc.PreInfinityExtrap = CompressedCurve.PreInfinityExtrap;
        CurveDesc.PostInfinityExtrap = CompressedCurve.PostInfinityExtrap;
        CurveDesc.ConstantValueNumKeys = CompressedCurve.ConstantValueNumKeys;
        CurveDesc.KeyDataOffset = KeyDataOffset;

        KeyDataOffset += CompressedCurve.CompressedKeys.Num();
        KeyData.Append(CompressedCurve.CompressedKeys);
    }

    TArray<uint8> TempBytes;
    TempBytes.Reserve(KeyDataOffset);

    // Serialize the compression settings into a temporary array. The archive
    // is flagged as persistent so that machines of different endianness produce
    // identical binary results.
    FMemoryWriter Ar(TempBytes, /*bIsPersistent=*/ true);

    Ar.Serialize(Curves.GetData(), sizeof(FCurveDesc) * NumCurves);
    Ar.Serialize(KeyData.GetData(), KeyData.Num());

    CompressedCurveBytes = TempBytes;
}

#if ENGINE_MAJOR_VERSION == 5


/////////////////////////////////////////////
////////////////////////////////////////////

UAnimSequence* FAnimSequenceFactory::CreateAnimSequence(USkeleton* Skeleton, ERichCurveInterpMode CurveInterpMode, FATLResponse* ATLOutput, const TArray<FPoseInfo>& PoseInfos, const TMap<FString, FString>& BoneNamesMapping, FString& Error, UObject* InParent /*= nullptr*/, const FString& AnimSequenceName/* = TEXT("")*/, bool bSetUpForMetahumans/* = false*/)
{
    check(ATLOutput != nullptr);

    // TODO: What if smart name mapping does not exist for the skeleton?
    const FSmartNameMapping* SmartNameMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
    ensure(SmartNameMapping != nullptr);

    bool bAutoSetTangents = true;
	
    check(ATLOutput->FPS > 0);
    double TimesecStep = 1.0f / ATLOutput->FPS;

#if WITH_EDITOR
    FScopedSlowTask SlowTask(ATLOutput->Frames.Num(), INVTEXT("Processing animation frames.."));
    SlowTask.MakeDialog();
#endif

    FRawCurveTracks RawCurveData;
    TMap<FName, FRawAnimSequenceTrack> BoneTracks;

    // Processing frames
    for (int32 FrameIndex = 0; FrameIndex < ATLOutput->Frames.Num(); ++FrameIndex)
    {
#if WITH_EDITOR
        SlowTask.EnterProgressFrame(1);
#endif

        const FATLFrameInfo& FrameInfo = ATLOutput->Frames[FrameIndex];
		
        // Apply pose mapping
        TMap<FName, float> CurveWeightedBlendShapeValue;
        for (int32 BlendShapeIndex = 0; BlendShapeIndex < ATLOutput->BlendShapesNames.Num(); ++BlendShapeIndex)
        {
            // find current pose info 
            FPoseInfo CurrentPoseInfo;
            for (const FPoseInfo& PoseInfo : PoseInfos)
            {
                if (PoseInfo.ATLResponse_BlendShapeIndex == BlendShapeIndex)
                {
                    CurrentPoseInfo = PoseInfo;
                    break;
                }
            }

            // fill current pose with default data if current blendshape not found in PoseInfos
            if (CurrentPoseInfo.ATLResponse_BlendShapeIndex == INDEX_NONE)
            {
                CurrentPoseInfo.PoseName = *ATLOutput->BlendShapesNames[BlendShapeIndex];
                CurrentPoseInfo.ATLResponse_BlendShapeIndex = BlendShapeIndex;
                CurrentPoseInfo.AffectedCurveNames.Add(CurrentPoseInfo.PoseName);
                CurrentPoseInfo.AffectedCurveWeights.Add(1.0f);
            }

            FSmartName SmartName;
            if (!SmartNameMapping->FindSmartName(CurrentPoseInfo.PoseName, SmartName))
            {
                if (BlendShapeIndex == 0) {
                    UE_LOG(LogMetahumanSDKAnimSequenceFactory, Warning, TEXT("Blendshape %s not found in provided skeleton!"), *CurrentPoseInfo.PoseName.ToString());
                }
                continue;
            }

            const float PoseWeight = FrameInfo.Blendshapes[BlendShapeIndex] / 100.0f;

            // for all poses related to current BlendShapeIndex calculate weighted blendshape value 
            for (int32 CurveIndex = 0; CurveIndex < CurrentPoseInfo.AffectedCurveNames.Num(); ++CurveIndex)
            {
                const FName CurveName = CurrentPoseInfo.AffectedCurveNames[CurveIndex];

                float CurrentBlendShapeValue = CurveWeightedBlendShapeValue.FindRef(CurveName) + CurrentPoseInfo.AffectedCurveWeights[CurveIndex] * PoseWeight;
                CurveWeightedBlendShapeValue.Add(CurveName, FMath::Clamp(CurrentBlendShapeValue, 0.0f, 1.0f));
            }
        }
		
        // Emulate ARKit MouthClose Pose
        if (bSetUpForMetahumans)
        {
            const float MouthCloseValue = FMath::Clamp(FrameInfo.Blendshapes[ATLOutput->BlendShapesNames.IndexOfByKey(TEXT("MouthClose"))] / 100.0f, 0.0f, 1.0f);
            const float JawOpenValue = FMath::Clamp(FrameInfo.Blendshapes[ATLOutput->BlendShapesNames.IndexOfByKey(TEXT("JawOpen"))] / 100.0f, 0.0f, 1.0f);

            float MouthLipsOverrideValue;
            if (JawOpenValue > KINDA_SMALL_NUMBER)
            {
                // MouthLipsOverrideValue = FMath::Clamp(MouthCloseValue * (1.0f - JawOpenValue), 0.0f, 1.0f);
                MouthLipsOverrideValue = FMath::Clamp(MouthCloseValue / JawOpenValue, 0.0f, 1.0f);
            }
            else
            {
                MouthLipsOverrideValue = FMath::Clamp(0.0f, 0.0f, 1.0f);
            }

            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherDL"), MouthLipsOverrideValue);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherDR"), MouthLipsOverrideValue);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherUL"), MouthLipsOverrideValue);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherUR"), MouthLipsOverrideValue);

            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyLPh1"), MouthLipsOverrideValue * 0.6f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyLPh2"), MouthLipsOverrideValue * 0.4f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyLPh3"), MouthLipsOverrideValue * 0.2f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyRPh1"), MouthLipsOverrideValue * 0.6f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyRPh2"), MouthLipsOverrideValue * 0.4f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyRPh3"), MouthLipsOverrideValue * 0.2f);

            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDC"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDINL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDINR"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDOUTL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDOUTR"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUC"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUINL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUINR"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUOUTL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUOUTR"), MouthLipsOverrideValue * 0.33f);
        }
		
        // set curve keys for current frame
        for (const TTuple<FName, float>& BlendShapeNameAndValue : CurveWeightedBlendShapeValue)
        {
            const FName CurveName = BlendShapeNameAndValue.Key;
            const float BlendShapeValue = BlendShapeNameAndValue.Value;

            FSmartName SmartName;
            if (!SmartNameMapping->FindSmartName(CurveName, SmartName))
            {
                UE_LOG(LogMetahumanSDKAnimSequenceFactory, Verbose, TEXT("Blendshape %s not found in provided skeleton!"), *CurveName.ToString());
                continue;
            }

            // construct curve if needed
            FFloatCurve* Curve = static_cast<FFloatCurve*>(RawCurveData.GetCurveData(SmartName.UID, ERawCurveTrackTypes::RCT_Float));
            if (Curve == nullptr)
            {
                ensure(RawCurveData.AddCurveData(SmartName));
                Curve = static_cast<FFloatCurve*>(RawCurveData.GetCurveData(SmartName.UID, ERawCurveTrackTypes::RCT_Float));
                Curve->Name = SmartName;
            }
            ensure(Curve != nullptr);

            // fill rich curve with keys
            FRichCurve& RichCurve = Curve->FloatCurve;

            FKeyHandle NewKeyHandle = RichCurve.AddKey(FrameIndex * TimesecStep, BlendShapeValue, false);

            ERichCurveTangentMode NewTangentMode = RCTM_Auto;
            ERichCurveTangentWeightMode NewTangentWeightMode = RCTWM_WeightedNone;

            // TODO: Set these values properly if we decide to support ERichCurveTangentMode and ERichCurveTangentWeightMode
            float LeaveTangent = 0.f;
            float ArriveTangent = 0.f;
            float LeaveTangentWeight = 0.0;
            float ArriveTangentWeight = 0.0;

            RichCurve.SetKeyInterpMode(NewKeyHandle, CurveInterpMode, bAutoSetTangents);
            RichCurve.SetKeyTangentMode(NewKeyHandle, NewTangentMode, bAutoSetTangents);
            RichCurve.SetKeyTangentWeightMode(NewKeyHandle, NewTangentWeightMode, bAutoSetTangents);

            FRichCurveKey& NewKey = RichCurve.GetKey(NewKeyHandle);
            NewKey.ArriveTangent = ArriveTangent;
            NewKey.LeaveTangent = LeaveTangent;
            NewKey.ArriveTangentWeight = ArriveTangentWeight;
            NewKey.LeaveTangentWeight = LeaveTangentWeight;
        }
		
        // Set bone keys for current frame
        const TArray<FTransform> BonesPoses = Skeleton->GetReferenceSkeleton().GetRefBonePose();
        for(int BoneIndex = 0; BoneIndex < ATLOutput->BonesNames.Num(); ++BoneIndex)
        {
            FString ATLBoneName = ATLOutput->BonesNames[BoneIndex];
            bool bHasBoneNameInMap = BoneNamesMapping.Contains(ATLBoneName) && BoneNamesMapping[ATLBoneName].Len();
            FString BoneNameString = bHasBoneNameInMap ? BoneNamesMapping[ATLBoneName] : ATLBoneName;
            FName BoneName = FName(BoneNameString);

            int32 BoneSkeletonIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName);
            if (BoneSkeletonIndex == INDEX_NONE)
            {
                continue;
            }
            if(!BoneTracks.Contains(BoneName))
            {
                BoneTracks.Add(BoneName, FRawAnimSequenceTrack());
            }

            const FVector& BoneTransform = FrameInfo.Bones[BoneIndex];
            FRawAnimSequenceTrack& Track = BoneTracks[BoneName];

            auto RotationDelta = FQuat::MakeFromEuler(BoneTransform);
            auto OriginalRotation = BonesPoses[BoneSkeletonIndex].GetRotation();
            auto Rotation = OriginalRotation * RotationDelta;

            Track.RotKeys.Add(FQuat4f(Rotation));
            Track.ScaleKeys.Add(FVector3f(BonesPoses[BoneSkeletonIndex].GetScale3D()));
            Track.PosKeys.Add(FVector3f(BonesPoses[BoneSkeletonIndex].GetLocation()));
        }
    }
	
    // Add 1 because of the way AnimSequence computes its framerate
    for(auto& Track : BoneTracks)
    {
        const TArray<FTransform> BonesPoses = Skeleton->GetReferenceSkeleton().GetRefBonePose();

        int32 BoneSkeletonIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(Track.Key);
        if (BoneSkeletonIndex != INDEX_NONE)
        {
            Track.Value.RotKeys.Add(FQuat4f(BonesPoses[BoneSkeletonIndex].GetRotation()));
            Track.Value.ScaleKeys.Add(FVector3f(BonesPoses[BoneSkeletonIndex].GetScale3D()));
            Track.Value.PosKeys.Add(FVector3f(BonesPoses[BoneSkeletonIndex].GetLocation()));
        }
    }
	
    // Add 1 because of the way AnimSequence computes its framerate
    const int32 NumFrames = ATLOutput->Frames.Num() + 1;
    const float SequenceLength = TimesecStep * ATLOutput->Frames.Num();
	
    // create UAnimSequence asset
    UPackage* AnimSequencePackage = Cast<UPackage>(InParent);
#if WITH_EDITOR
    // we have to check InParent is not valid as this will tell us whether we import asset via editor asset factory or not
    if (MetahumanSDKDevSettings::bCreateAssetsInEditor && !IsValid(InParent))
    {
        FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

        FString DefaultSuffix;
        FString AssetName;
        FString PackageName;
        const FString BasePackageName(FString::Printf(TEXT("/MetahumanSDK/DebugAssets/ATL/AnimSequence"), *AnimSequenceName));
        AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);

        // Create a unique package name and asset name
        AnimSequencePackage = CreatePackage(*PackageName);
        AnimSequencePackage->SetFlags(RF_Standalone | RF_Public);
        AnimSequencePackage->FullyLoad();
    }
#endif

    UAnimSequence* AnimSequence = NewObject<UAnimSequence>(IsValid(AnimSequencePackage) ? AnimSequencePackage : GetTransientPackage(), *AnimSequenceName, RF_Public | RF_Standalone);    
    {
        AnimSequence->SetSkeleton(Skeleton);
        AnimSequence->SetSequenceLength(SequenceLength);
    }
	
#if WITH_EDITOR
    // Create in-editor data
    AnimSequence->GetController().OpenBracket(LOCTEXT("MetahumanSDKAnimSequenceFactory", "Adding float curve."), false);
    AnimSequence->GetController().SetFrameRate(FFrameRate(ATLOutput->FPS, 1.f));
    AnimSequence->GetController().SetPlayLength(SequenceLength);
	
    for(auto FloatCurve: RawCurveData.FloatCurves)
    {
        const FAnimationCurveIdentifier CurveId(FloatCurve.Name, ERawCurveTrackTypes::RCT_Float);
        AnimSequence->GetController().AddCurve(CurveId, FloatCurve.GetCurveTypeFlags());
        AnimSequence->GetController().SetCurveKeys(CurveId, FloatCurve.FloatCurve.Keys);
    }
	
    for(auto& Track : BoneTracks)
    {
        AnimSequence->GetController().AddBoneTrack(Track.Key);
        AnimSequence->GetController().SetBoneTrackKeys(Track.Key, Track.Value.PosKeys, Track.Value.RotKeys, Track.Value.ScaleKeys);
    }
	
    AnimSequence->GetController().NotifyPopulated();
    AnimSequence->GetController().CloseBracket();
#else
    // Prepare animation compression payload
    FCompressibleAnimData CompressibleAnimData(
        nullptr,
        FAnimationUtils::GetDefaultAnimationCurveCompressionSettings(),
        Skeleton,
        EAnimInterpolationType::Linear,
        SequenceLength,
        NumFrames
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
        , nullptr
#endif
        );
    CompressibleAnimData.RawFloatCurves = RawCurveData.FloatCurves;

    // Compress blendshapes
    TArray<uint8> CompressedCurveData;
    CompressCurves(CompressibleAnimData, CompressedCurveData);
    AnimSequence->CompressedData.CurveCompressionCodec = CompressibleAnimData.CurveCompressionSettings->Codec;
    AnimSequence->CompressedData.CompressedCurveByteStream = CompressedCurveData;
    AnimSequence->CompressedData.CompressedCurveNames.Empty();
    for(auto FloatCurve: RawCurveData.FloatCurves)
    {
        AnimSequence->CompressedData.CompressedCurveNames.Add(FloatCurve.Name);
    }

    // create runtime codec
    auto Codec = NewObject<UMetahumanSDKBoneCompressionCodec>();
    BoneTracks.GenerateValueArray(Codec->Tracks);
    Codec->NumberOfKeys = CompressibleAnimData.NumberOfKeys;

    // make compression settings with this codec only and bind it to asset
    AnimSequence->BoneCompressionSettings = NewObject<UAnimBoneCompressionSettings>();
    AnimSequence->BoneCompressionSettings->Codecs.Add(Codec);
    AnimSequence->CompressedData.BoneCompressionCodec = Codec;

    // fake-fill some compressed data to pass model validation
    FUECompressedAnimDataMutable AnimDataMutable;
    AnimDataMutable.CompressedNumberOfKeys = CompressibleAnimData.NumberOfKeys;
    AnimSequence->CompressedData.CompressedDataStructure = Codec->AllocateAnimData();

    AnimSequence->CompressedData.CompressedTrackToSkeletonMapTable.Empty();
    for (auto &Track: BoneTracks)
    {
        AnimSequence->CompressedData.CompressedTrackToSkeletonMapTable.Add(
            AnimSequence->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Track.Key));
    }
#endif

	return AnimSequence;
}






/////////////////////////////////////////////
////////////////////////////////////////////


#else
UAnimSequence* FAnimSequenceFactory::CreateAnimSequence(USkeleton* Skeleton, ERichCurveInterpMode CurveInterpMode, FATLResponse* ATLOutput, const TArray<FPoseInfo>& PoseInfos, const TMap<FString, FString>& BoneNamesMapping, FString& Error, UObject* InParent /*= nullptr*/, const FString& AnimSequenceName/* = TEXT("")*/, bool bSetUpForMetahumans/* = false*/)
{
    check(ATLOutput != nullptr);

    // TODO: What if smart name mapping does not exist for the skeleton?
    const FSmartNameMapping* SmartNameMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
    ensure(SmartNameMapping != nullptr);

    bool bAutoSetTangents = true;
    double TimesecStep = 1.0f / ATLOutput->FPS; 

    // curve tracks container
    FRawCurveTracks RawCurveData;

#if WITH_EDITOR
    FScopedSlowTask SlowTask(ATLOutput->Frames.Num(), INVTEXT("Processing animation frames.."));
    SlowTask.MakeDialog();
#endif

    TMap<FName, FRawAnimSequenceTrack> Tracks;

    for (int32 FrameIndex = 0; FrameIndex < ATLOutput->Frames.Num(); ++FrameIndex)
    {
#if WITH_EDITOR
        SlowTask.EnterProgressFrame(1);
#endif
        
        const FATLFrameInfo& FrameInfo = ATLOutput->Frames[FrameIndex];

        TMap<FName, float> CurveWeightedBlendShapeValue;
        TMap<FString, FTransform> TrackValue;

        for (int32 BlendShapeIndex = 0; BlendShapeIndex < ATLOutput->BlendShapesNames.Num(); ++BlendShapeIndex)
        {
            // find current pose info 
            FPoseInfo CurrentPoseInfo;
            for (const FPoseInfo& PoseInfo : PoseInfos)
            {
                if (PoseInfo.ATLResponse_BlendShapeIndex == BlendShapeIndex)
                {
                    CurrentPoseInfo = PoseInfo;
                    break;
                }
            }

            // fill current pose with default data if current blendshape not found in PoseInfos
            if (CurrentPoseInfo.ATLResponse_BlendShapeIndex == INDEX_NONE)
            {
                CurrentPoseInfo.PoseName = *ATLOutput->BlendShapesNames[BlendShapeIndex];
                CurrentPoseInfo.ATLResponse_BlendShapeIndex = BlendShapeIndex;
                CurrentPoseInfo.AffectedCurveNames.Add(CurrentPoseInfo.PoseName);
                CurrentPoseInfo.AffectedCurveWeights.Add(1.0f);
            }

            FSmartName SmartName;
            if (!SmartNameMapping->FindSmartName(CurrentPoseInfo.PoseName, SmartName))
            {
                UE_LOG(LogMetahumanSDKAnimSequenceFactory, Warning, TEXT("Blendshape %s not found in provided skeleton!"), *CurrentPoseInfo.PoseName.ToString());
                continue;
            }

            const float PoseWeight = FrameInfo.Blendshapes[BlendShapeIndex] / 100.0f;

            // for all poses related to current BlendShapeIndex calculate weighted blendshape value 
            for (int32 CurveIndex = 0; CurveIndex < CurrentPoseInfo.AffectedCurveNames.Num(); ++CurveIndex)
            {
                const FName CurveName = CurrentPoseInfo.AffectedCurveNames[CurveIndex];

                float CurrentBlendShapeValue = CurveWeightedBlendShapeValue.FindRef(CurveName) + CurrentPoseInfo.AffectedCurveWeights[CurveIndex] * PoseWeight;
                CurveWeightedBlendShapeValue.Add(CurveName, FMath::Clamp(CurrentBlendShapeValue, 0.0f, 1.0f));
            }
        }

        // Emulate ARKit MouthClose Pose
        if (bSetUpForMetahumans)
        {
            const float MouthCloseValue = FMath::Clamp(FrameInfo.Blendshapes[ATLOutput->BlendShapesNames.IndexOfByKey(TEXT("MouthClose"))] / 100.0f, 0.0f, 1.0f);
            const float JawOpenValue = FMath::Clamp(FrameInfo.Blendshapes[ATLOutput->BlendShapesNames.IndexOfByKey(TEXT("JawOpen"))] / 100.0f, 0.0f, 1.0f);

            float MouthLipsOverrideValue;
            if (JawOpenValue > KINDA_SMALL_NUMBER)
            {
                // MouthLipsOverrideValue = FMath::Clamp(MouthCloseValue * (1.0f - JawOpenValue), 0.0f, 1.0f);
                MouthLipsOverrideValue = FMath::Clamp(MouthCloseValue / JawOpenValue, 0.0f, 1.0f);
            }
            else
            {
                MouthLipsOverrideValue = FMath::Clamp(0.0f, 0.0f, 1.0f);
            }

            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherDL"), MouthLipsOverrideValue);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherDR"), MouthLipsOverrideValue);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherUL"), MouthLipsOverrideValue);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTogetherUR"), MouthLipsOverrideValue);

            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyLPh1"), MouthLipsOverrideValue * 0.6f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyLPh2"), MouthLipsOverrideValue * 0.4f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyLPh3"), MouthLipsOverrideValue * 0.2f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyRPh1"), MouthLipsOverrideValue * 0.6f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyRPh2"), MouthLipsOverrideValue * 0.4f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsStickyRPh3"), MouthLipsOverrideValue * 0.2f);

            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDC"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDINL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDINR"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDOUTL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyDOUTR"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUC"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUINL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUINR"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUOUTL"), MouthLipsOverrideValue * 0.33f);
            CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthStickyUOUTR"), MouthLipsOverrideValue * 0.33f);

            // const float MouthLipsPurseValue = FMath::Clamp(3.0f * FrameInfo.Blendshapes[ATLOutput->BlendShapesNames.IndexOfByKey(TEXT("MouthPucker"))] / 100.0f, 0.0f, 1.0f);
            
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsPurseDL"), MouthLipsPurseValue);
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsPurseDR"), MouthLipsPurseValue);
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsPurseUL"), MouthLipsPurseValue);
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsPurseUR"), MouthLipsPurseValue);
            //
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTowardsDL"), MouthLipsPurseValue);
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTowardsDR"), MouthLipsPurseValue);
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTowardsUL"), MouthLipsPurseValue);
            // CurveWeightedBlendShapeValue.Add(TEXT("CTRL_expressions_mouthLipsTowardsUR"), MouthLipsPurseValue);
        }
        
        // set curve keys for current frame
        for (const TTuple<FName, float>& BlendShapeNameAndValue : CurveWeightedBlendShapeValue)
        {
            const FName CurveName = BlendShapeNameAndValue.Key;
            const float BlendShapeValue = BlendShapeNameAndValue.Value;

            FSmartName SmartName;
            if (!SmartNameMapping->FindSmartName(CurveName, SmartName))
            {
                UE_LOG(LogMetahumanSDKAnimSequenceFactory, Verbose, TEXT("Blendshape %s not found in provided skeleton!"), *CurveName.ToString());
                continue;
            }

            // construct curve if needed
            FFloatCurve* Curve = static_cast<FFloatCurve*>(RawCurveData.GetCurveData(SmartName.UID, ERawCurveTrackTypes::RCT_Float));
            if (Curve == nullptr)
            {
                ensure(RawCurveData.AddCurveData(SmartName));
                Curve = static_cast<FFloatCurve*>(RawCurveData.GetCurveData(SmartName.UID, ERawCurveTrackTypes::RCT_Float));
                Curve->Name = SmartName;
            }
            ensure(Curve != nullptr);

            // fill rich curve with keys
            FRichCurve& RichCurve = Curve->FloatCurve;

            FKeyHandle NewKeyHandle = RichCurve.AddKey(FrameIndex * TimesecStep, BlendShapeValue, false);

            ERichCurveTangentMode NewTangentMode = RCTM_Auto;
            ERichCurveTangentWeightMode NewTangentWeightMode = RCTWM_WeightedNone;

            // TODO: Set these values properly if we decide to support ERichCurveTangentMode and ERichCurveTangentWeightMode
            float LeaveTangent = 0.f;
            float ArriveTangent = 0.f;
            float LeaveTangentWeight = 0.0;
            float ArriveTangentWeight = 0.0;

            RichCurve.SetKeyInterpMode(NewKeyHandle, CurveInterpMode, bAutoSetTangents);
            RichCurve.SetKeyTangentMode(NewKeyHandle, NewTangentMode, bAutoSetTangents);
            RichCurve.SetKeyTangentWeightMode(NewKeyHandle, NewTangentWeightMode, bAutoSetTangents);

            FRichCurveKey& NewKey = RichCurve.GetKey(NewKeyHandle);
            NewKey.ArriveTangent = ArriveTangent;
            NewKey.LeaveTangent = LeaveTangent;
            NewKey.ArriveTangentWeight = ArriveTangentWeight;
            NewKey.LeaveTangentWeight = LeaveTangentWeight;
        }

        const TArray<FTransform> BonesPoses = Skeleton->GetReferenceSkeleton().GetRefBonePose();
        for(int BoneIndex = 0; BoneIndex < ATLOutput->BonesNames.Num(); ++BoneIndex)
        {
            FString ATLBoneName = ATLOutput->BonesNames[BoneIndex];
            bool bHasBoneNameInMap = BoneNamesMapping.Contains(ATLBoneName) && BoneNamesMapping[ATLBoneName].Len();
            FString BoneNameString = bHasBoneNameInMap ? BoneNamesMapping[ATLBoneName] : ATLBoneName;
            FName BoneName = FName(BoneNameString);

            int32 BoneSkeletonIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName);
            if (BoneSkeletonIndex == INDEX_NONE)
            {
                continue;
            }
            if(!Tracks.Contains(BoneName))
            {
                Tracks.Add(BoneName, FRawAnimSequenceTrack());
            }

            const FVector& BoneTransform = FrameInfo.Bones[BoneIndex];
            FRawAnimSequenceTrack& Track = Tracks[BoneName];

            auto RotationDelta = FQuat::MakeFromEuler(BoneTransform);
            auto OriginalRotation = BonesPoses[BoneSkeletonIndex].GetRotation();
            auto Rotation = OriginalRotation * RotationDelta;
            
            Track.RotKeys.Add(Rotation);
            Track.ScaleKeys.Add(BonesPoses[BoneSkeletonIndex].GetScale3D());
            Track.PosKeys.Add(BonesPoses[BoneSkeletonIndex].GetLocation());
        }
    }

    // Add 1 because of the way AnimSequence computes its framerate
    for(auto& Track : Tracks)
    {
        const TArray<FTransform> BonesPoses = Skeleton->GetReferenceSkeleton().GetRefBonePose();

        int32 BoneSkeletonIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(Track.Key);
        if (BoneSkeletonIndex != INDEX_NONE)
        {
            Track.Value.RotKeys.Add(BonesPoses[BoneSkeletonIndex].GetRotation());
            Track.Value.ScaleKeys.Add(BonesPoses[BoneSkeletonIndex].GetScale3D());
            Track.Value.PosKeys.Add(BonesPoses[BoneSkeletonIndex].GetLocation());
        }
    }

    // Add 1 because of the way AnimSequence computes its framerate
    const int32 NumFrames = ATLOutput->Frames.Num() + 1;
    const float SequenceLength = TimesecStep * ATLOutput->Frames.Num();


    // runtime compress RawCurveData
    FCompressibleAnimData CompressibleAnimData;
    {
        CompressibleAnimData.CurveCompressionSettings = FAnimationUtils::GetDefaultAnimationCurveCompressionSettings();
        CompressibleAnimData.RawCurveData = RawCurveData;
        CompressibleAnimData.NumFrames = NumFrames;
        CompressibleAnimData.SequenceLength = SequenceLength;
    }
    TArray<uint8> CompressedCurveData;
    CompressCurves(CompressibleAnimData, CompressedCurveData);

    // create UAnimSequence asset
    UPackage* AnimSequencePackage = Cast<UPackage>(InParent);

#if WITH_EDITOR
    // we have to check InParent is not valid as this will tell us whether we import asset via editor asset factory or not
    if (MetahumanSDKDevSettings::bCreateAssetsInEditor && !IsValid(InParent))
    {
        FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

        FString DefaultSuffix;
        FString AssetName;
        FString PackageName;
        const FString BasePackageName(FString::Printf(TEXT("/MetahumanSDK/DebugAssets/ATL/AnimSequence"), *AnimSequenceName));
        AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);

        // Create a unique package name and asset name
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
        AnimSequencePackage = CreatePackage(nullptr, *PackageName);
#else
        AnimSequencePackage = CreatePackage(*PackageName);
#endif

        AnimSequencePackage->SetFlags(RF_Standalone | RF_Public);
        AnimSequencePackage->FullyLoad();
    }
#endif

    UAnimSequence* AnimSequence = NewObject<UAnimSequence>(IsValid(AnimSequencePackage) ? AnimSequencePackage : GetTransientPackage(), *AnimSequenceName, RF_Public | RF_Standalone);
   
    AnimSequence->SetSkeleton(Skeleton);
    {
        AnimSequence->SetRawNumberOfFrame(NumFrames);
        AnimSequence->SequenceLength = SequenceLength;
    }

#if !WITH_EDITOR
    UMetahumanSDKBoneCompressionCodec* BoneCompressionCodec = NewObject<UMetahumanSDKBoneCompressionCodec>();

    const TArray<FTransform> BonesPoses = AnimSequence->GetSkeleton()->GetReferenceSkeleton().GetRefBonePose();
    BoneCompressionCodec->Tracks.AddDefaulted(BonesPoses.Num());
    AnimSequence->CompressedData.CompressedTrackToSkeletonMapTable.AddDefaulted(BonesPoses.Num());
    for (int32 BoneIndex = 0; BoneIndex < BonesPoses.Num(); BoneIndex++)
    {
        for (int32 FrameIndex = 0; FrameIndex < NumFrames; FrameIndex++)
        {
            AnimSequence->CompressedData.CompressedTrackToSkeletonMapTable[BoneIndex] = BoneIndex;

            BoneCompressionCodec->Tracks[BoneIndex].PosKeys.Add(BonesPoses[BoneIndex].GetLocation());
            BoneCompressionCodec->Tracks[BoneIndex].RotKeys.Add(BonesPoses[BoneIndex].GetRotation());
            BoneCompressionCodec->Tracks[BoneIndex].ScaleKeys.Add(BonesPoses[BoneIndex].GetScale3D());
        }
    }

#endif
    
    for (auto& Pair : Tracks)
    {
#if WITH_EDITOR
        AnimSequence->AddNewRawTrack(Pair.Key, &Pair.Value);
#else
        int32 BoneIndex = AnimSequence->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Pair.Key);
        BoneCompressionCodec->Tracks[BoneIndex] = Pair.Value;
#endif
    }

    // prepare AnimSequence compressed data for runtime
    AnimSequence->CompressedData.CompressedCurveByteStream = CompressedCurveData;
    AnimSequence->CompressedData.CurveCompressionCodec = FAnimationUtils::GetDefaultAnimationCurveCompressionSettings()->Codec;
    AnimSequence->CompressedData.CompressedRawDataSize = CompressibleAnimData.GetApproxRawSize();
    {
        const int32 NumCurves = RawCurveData.FloatCurves.Num();
        AnimSequence->CompressedData.CompressedCurveNames.Empty(NumCurves);
        for (int32 CurveIndex = 0; CurveIndex < NumCurves; ++CurveIndex)
        {
            const FFloatCurve& Curve = RawCurveData.FloatCurves[CurveIndex];
            AnimSequence->CompressedData.CompressedCurveNames.Add(Curve.Name);
        }
    }

#if WITH_EDITOR
    // make it possible to edit blendshapes (curve tracks) in editor
    AnimSequence->RawCurveData = RawCurveData;

    // we have to check InParent is not valid as this will tell us whether we import asset via editor asset factory or not
    if (MetahumanSDKDevSettings::bCreateAssetsInEditor && !IsValid(InParent))
    {
        AnimSequence->SetUseRawDataOnly(true);
        AnimSequence->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(AnimSequence);
    }
    AnimSequence->PostProcessSequence();
#else
    AnimSequence->CompressedData.BoneCompressionCodec = BoneCompressionCodec;
    AnimSequence->PostLoad();
#endif

    return AnimSequence;
}
#endif

void FAnimSequenceFactory::ConvertMappingsAssetToPoseInfos(FATLResponse* ATLResponse, const FATLMappingsInfo& Mappings, TArray<FPoseInfo>& OutPoseInfos)
{
    const UMetahumanSDKMappingsAsset* MappingsAsset = Mappings.GetMappingsAsset();
    const UPoseAsset* PoseAsset = Mappings.GetPoseAsset();    

    // handle mappings asset
    if (IsValid(MappingsAsset))
    {
        for (int32 BlendShapeIndex = 0; BlendShapeIndex < ATLResponse->BlendShapesNames.Num(); ++BlendShapeIndex)
        {
            const FName BlendShapeName = *ATLResponse->BlendShapesNames[BlendShapeIndex];

            FPoseInfo PoseInfo;
            {
                const FName MappingTargetValue = *MappingsAsset->BlendShapeMappings.FindRef(BlendShapeName.ToString());

                if (MappingsAsset->BlendShapeMappings.Contains(BlendShapeName.ToString()) && MappingTargetValue.ToString().Len() > 0 && MappingTargetValue != BlendShapeName)
                {
                    // collect curves affected by pose
                    PoseInfo.PoseName = MappingTargetValue;
                    PoseInfo.AffectedCurveNames.Add(PoseInfo.PoseName);
                    PoseInfo.AffectedCurveWeights.Add(1.0f);
                }
                else
                {
                    // collect curves affected by pose
                    PoseInfo.PoseName = BlendShapeName;
                    PoseInfo.AffectedCurveNames.Add(BlendShapeName);
                    PoseInfo.AffectedCurveWeights.Add(1.0f);
                }

                PoseInfo.ATLResponse_BlendShapeIndex = BlendShapeIndex;

                check(PoseInfo.AffectedCurveNames.Num() == PoseInfo.AffectedCurveWeights.Num());
            }

            OutPoseInfos.Add(PoseInfo);
        }
    }
    // handle pose asset
    else if (IsValid(PoseAsset))
    {
        const TArray<FSmartName>& CurveNames = PoseAsset->GetCurveNames();
        const TArray<FSmartName>& PoseNames = PoseAsset->GetPoseNames();
        
        TMap<FName, int32> PoseNamesFastLookup;
        for (int32 PoseIndex = 0; PoseIndex < PoseNames.Num(); ++PoseIndex)
        {
            PoseNamesFastLookup.Add(PoseNames[PoseIndex].DisplayName, PoseIndex);
        }

        for (int32 BlendShapeIndex = 0; BlendShapeIndex < ATLResponse->BlendShapesNames.Num(); ++BlendShapeIndex)
        {
            const FName BlendShapeName = *ATLResponse->BlendShapesNames[BlendShapeIndex];

            FPoseInfo PoseInfo;
            {
                if (PoseNamesFastLookup.Contains(BlendShapeName))
                {
                    const FName& PoseName = BlendShapeName;
                    const int32 PoseIndex = PoseNamesFastLookup[BlendShapeName];

                    // collect curves affected by pose
                    PoseInfo.PoseName = PoseName;

                    const TArray<float>& CurveValues = PoseAsset->GetCurveValues(PoseIndex);
                    for (int32 CurveIndex = 0; CurveIndex < CurveValues.Num(); ++CurveIndex)
                    {
                        if (CurveValues[CurveIndex] > 0)
                        {
                            PoseInfo.AffectedCurveNames.Add(CurveNames[CurveIndex].DisplayName);
                            PoseInfo.AffectedCurveWeights.Add(CurveValues[CurveIndex]);
                        }
                    }
                }
                else
                {
                    PoseInfo.PoseName = BlendShapeName;
                    PoseInfo.AffectedCurveNames.Add(BlendShapeName);
                    PoseInfo.AffectedCurveWeights.Add(1.0f);
                }
                PoseInfo.ATLResponse_BlendShapeIndex = BlendShapeIndex;

                check(PoseInfo.AffectedCurveNames.Num() == PoseInfo.AffectedCurveWeights.Num());
            }

            OutPoseInfos.Add(PoseInfo);
        }
    }
}
