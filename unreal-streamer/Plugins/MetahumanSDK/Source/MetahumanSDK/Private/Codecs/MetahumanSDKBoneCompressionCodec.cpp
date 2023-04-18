#include "Codecs/MetahumanSDKBoneCompressionCodec.h"

UMetahumanSDKBoneCompressionCodec::UMetahumanSDKBoneCompressionCodec() = default;
UMetahumanSDKBoneCompressionCodec::~UMetahumanSDKBoneCompressionCodec() = default;

void UMetahumanSDKBoneCompressionCodec::DecompressBone(FAnimSequenceDecompressionContext& DecompContext, int32 TrackIndex, FTransform& OutAtom) const
{
	OutAtom.SetLocation(GetTrackLocation(DecompContext, TrackIndex));
	OutAtom.SetRotation(GetTrackRotation(DecompContext, TrackIndex));
	OutAtom.SetScale3D(GetTrackScale(DecompContext, TrackIndex));
}

FQuat UMetahumanSDKBoneCompressionCodec::GetTrackRotation(FAnimSequenceDecompressionContext& DecompContext, const int32 TrackIndex) const
{
	int32 FrameA = 0;
	int32 FrameB = 0;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	float SequenceLength = DecompContext.GetPlayableLength();
	float DecompContextRelPos = DecompContext.GetRelativePosition();
#else // UE < 5.1
	float SequenceLength = DecompContext.SequenceLength;
	float DecompContextRelPos = DecompContext.RelativePos;
#endif
	
	double Alpha = TimeToIndex(SequenceLength, DecompContextRelPos, Tracks[TrackIndex].RotKeys.Num(), DecompContext.Interpolation, FrameA, FrameB);
	return FQuat::Slerp(FQuat(Tracks[TrackIndex].RotKeys[FrameA]), FQuat(Tracks[TrackIndex].RotKeys[FrameB]), Alpha);
}

FVector UMetahumanSDKBoneCompressionCodec::GetTrackLocation(FAnimSequenceDecompressionContext& DecompContext, const int32 TrackIndex) const
{
	int32 FrameA = 0;
	int32 FrameB = 0;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	float SequenceLength = DecompContext.GetPlayableLength();
	float DecompContextRelPos = DecompContext.GetRelativePosition();
#else // UE < 5.1
	float SequenceLength = DecompContext.SequenceLength;
	float DecompContextRelPos = DecompContext.RelativePos;
#endif
	
	double Alpha = TimeToIndex(SequenceLength, DecompContextRelPos, Tracks[TrackIndex].PosKeys.Num(), DecompContext.Interpolation, FrameA, FrameB);
	return FMath::Lerp(FVector(Tracks[TrackIndex].PosKeys[FrameA]), FVector(Tracks[TrackIndex].PosKeys[FrameB]), Alpha);
}

FVector UMetahumanSDKBoneCompressionCodec::GetTrackScale(FAnimSequenceDecompressionContext& DecompContext, const int32 TrackIndex) const
{
	int32 FrameA = 0;
	int32 FrameB = 0;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	float SequenceLength = DecompContext.GetPlayableLength();
	float DecompContextRelPos = DecompContext.GetRelativePosition();
#else // UE < 5.1
	float SequenceLength = DecompContext.SequenceLength;
	float DecompContextRelPos = DecompContext.RelativePos;
#endif
	
	double Alpha = TimeToIndex(SequenceLength, DecompContextRelPos, Tracks[TrackIndex].ScaleKeys.Num(), DecompContext.Interpolation, FrameA, FrameB);
	return FMath::Lerp(FVector(Tracks[TrackIndex].ScaleKeys[FrameA]), FVector(Tracks[TrackIndex].ScaleKeys[FrameB]), Alpha);
}

void UMetahumanSDKBoneCompressionCodec::DecompressPose(FAnimSequenceDecompressionContext& DecompContext, const BoneTrackArray& RotationPairs, const BoneTrackArray& TranslationPairs, const BoneTrackArray& ScalePairs, TArrayView<FTransform>& OutAtoms) const
{
	for (const BoneTrackPair& BoneTrackPair : RotationPairs)
	{
		OutAtoms[BoneTrackPair.AtomIndex].SetRotation(GetTrackRotation(DecompContext, BoneTrackPair.TrackIndex));
	}

	for (const BoneTrackPair& BoneTrackPair : TranslationPairs)
	{
		OutAtoms[BoneTrackPair.AtomIndex].SetLocation(GetTrackLocation(DecompContext, BoneTrackPair.TrackIndex));
	}

	for (const BoneTrackPair& BoneTrackPair : ScalePairs)
	{
		OutAtoms[BoneTrackPair.AtomIndex].SetScale3D(GetTrackScale(DecompContext, BoneTrackPair.TrackIndex));
	}
}

UAnimBoneCompressionCodec* UMetahumanSDKBoneCompressionCodec::GetCodec(const FString& DDCHandle)
{
	return this;
}

TUniquePtr<ICompressedAnimData> UMetahumanSDKBoneCompressionCodec::AllocateAnimData() const
{
	// FUECompressedAnimDataMutable AnimDataMutable;
	// AnimDataMutable.CompressedNumberOfKeys = NumberOfKeys;
	// return TUniquePtr<ICompressedAnimData>(&AnimDataMutable);

	auto AnimData = MakeUnique<FUECompressedAnimData>();
	AnimData->CompressedNumberOfKeys = NumberOfKeys;
	return AnimData;
}

// Taken from official Unreal Engine code base.
double UMetahumanSDKBoneCompressionCodec::TimeToIndex(
	float SequenceLength,
	float RelativePos,
	int32 NumKeys,
	EAnimInterpolationType Interpolation,
	int32& PosIndex0Out,
	int32& PosIndex1Out) const
{
	double Alpha;

	if (NumKeys < 2)
	{
		checkSlow(NumKeys == 1); // check if data is empty for some reason.
		PosIndex0Out = 0;
		PosIndex1Out = 0;
		return 0.0f;
	}
	// Check for before-first-frame case.
	if (RelativePos <= 0.f)
	{
		PosIndex0Out = 0;
		PosIndex1Out = 0;
		Alpha = 0.0f;
	}
	else
	{
		NumKeys -= 1; // never used without the minus one in this case
		// Check for after-last-frame case.
		if (RelativePos >= 1.0f)
		{
			// If we're not looping, key n-1 is the final key.
			PosIndex0Out = NumKeys;
			PosIndex1Out = NumKeys;
			Alpha = 0.0f;
		}
		else
		{
			// For non-looping animation, the last frame is the ending frame, and has no duration.
			const float KeyPos = RelativePos * float(NumKeys);
			checkSlow(KeyPos >= 0.0f);
			const float KeyPosFloor = floorf(KeyPos);
			PosIndex0Out = FMath::Min(FMath::TruncToInt(KeyPosFloor), NumKeys);
			Alpha = (Interpolation == EAnimInterpolationType::Step) ? 0.0f : KeyPos - KeyPosFloor;
			PosIndex1Out = FMath::Min(PosIndex0Out + 1, NumKeys);
		}
	}
	return Alpha;
}