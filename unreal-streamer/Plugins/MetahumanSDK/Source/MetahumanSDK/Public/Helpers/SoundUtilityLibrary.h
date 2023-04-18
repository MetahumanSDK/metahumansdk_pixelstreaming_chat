#pragma once

#include "CoreMinimal.h"


class USoundWave;
class USoundWaveProcedural;
struct FSoundQualityInfo;


struct FSoundDescription
{
	FSoundDescription(){};
	
	FSoundDescription(const uint32 InNumChannels, const uint32 InSampleRate, const uint32 InSampleDataSize,	const bool bInBitPerSample32) :
		NumChannels(InNumChannels), SampleRate(InSampleRate), SampleDataSize(InSampleDataSize), bBitPerSample32(bInBitPerSample32) {}

	FSoundDescription(const uint32 InNumChannels, const uint32 InSampleRate, const uint32 InSampleDataSize) :
				NumChannels(InNumChannels), SampleRate(InSampleRate), SampleDataSize(InSampleDataSize)	{}
	
	uint32 NumChannels = -1;
	
	uint32 SampleRate = -1;
	
	uint32 SampleDataSize = -1;

	bool bBitPerSample32 = false;
};

class METAHUMANSDK_API FSoundUtilityLibrary
{
public:

	static bool GetSoundWaveData(USoundWave* InSoundWave, TArray<uint8>& OutData);

	static bool ApplyCompressedData(const FName AudioFormatName, const TArray<uint8>& CompressedData, USoundWave* InSoundWave);
	
private:

	static bool PrepareProceduralSoundWave(USoundWaveProcedural* InProceduralSoundWave);
	
	static bool GetCookedPlatformData(const USoundWave* InSoundWave, const FString& AudioFormat, TArray<uint8>& OutData);
	
#if WITH_EDITOR	
	static bool GetEditorSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData);
#else
	// Get Sound data in the packaged game
	static bool GetPackagedSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData);
	
	static bool GetChunkedSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData);
	static bool IsNeedDecompress(const USoundWave* InSoundWave);
	static FName GetSoundRuntimeFormat(const USoundWave* InSoundWave);
	static bool DecompressSoundData(const TArray<uint8>& InData, FSoundQualityInfo& OutSoundQuality, TArray<uint8>& OutData);
#endif
	
	static bool EncodePcmToWav(const TArray<uint8>& PcmData, const FSoundQualityInfo SoundQuality, TArray<uint8>& WavData);
	static bool EncodePcmToWav(const void* PcmDataPtr, const FSoundQualityInfo SoundQuality, TArray<uint8>& WavData);
	static bool EncodePcmToWav(const void* PcmDataPtr, const FSoundDescription SoundDescription, TArray<uint8>& WavData);
	
	// Deprecated since UE5
	static bool GetFormatSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData);	
	static bool GetCompressedSoundFormatName(const USoundWave* InSoundWave, FName& CompressedSoundFormat);
	
};


