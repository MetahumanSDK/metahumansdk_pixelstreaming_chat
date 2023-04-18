#pragma once

#include "CoreMinimal.h"


class USoundWave;
struct FSoundQualityInfo;

class METAHUMANSDK_API FSoundWaveFactory
{
public:
    static USoundWave* CreateSoundWave(const TArray<uint8>& RawData, FString& Error, UObject* InParent = nullptr, const FString& SoundWaveName = TEXT(""));

    static bool CookRawAudio(const TArray<uint8>& SrcBuffer, FSoundQualityInfo& QualityInfo, TArray<uint8>& OutData);
};