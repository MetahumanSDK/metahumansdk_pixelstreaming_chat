#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimBoneCompressionCodec.h"
#include "MetahumanSDKBoneCompressionCodec.generated.h"

UCLASS()
class METAHUMANSDK_API UMetahumanSDKBoneCompressionCodec : public UAnimBoneCompressionCodec
{
	GENERATED_BODY()

public:
	UMetahumanSDKBoneCompressionCodec();
	~UMetahumanSDKBoneCompressionCodec();

	virtual void DecompressBone(FAnimSequenceDecompressionContext& DecompContext, int32 TrackIndex, FTransform& OutAtom) const;
	virtual void DecompressPose(FAnimSequenceDecompressionContext& DecompContext, const BoneTrackArray& RotationPairs, const BoneTrackArray& TranslationPairs, const BoneTrackArray& ScalePairs, TArrayView<FTransform>& OutAtoms) const;
	virtual UAnimBoneCompressionCodec* GetCodec(const FString& DDCHandle) override;
	virtual TUniquePtr<ICompressedAnimData> AllocateAnimData() const override;

	TArray<FRawAnimSequenceTrack> Tracks;
	int32 NumberOfKeys;
protected:
	double TimeToIndex(
		float SequenceLength,
		float RelativePos,
		int32 NumKeys,
		EAnimInterpolationType Interpolation,
		int32& PosIndex0Out,
		int32& PosIndex1Out) const;

	FQuat GetTrackRotation(FAnimSequenceDecompressionContext& DecompContext, const int32 TrackIndex) const;
	FVector GetTrackLocation(FAnimSequenceDecompressionContext& DecompContext, const int32 TrackIndex) const;
	FVector GetTrackScale(FAnimSequenceDecompressionContext& DecompContext, const int32 TrackIndex) const;
};
