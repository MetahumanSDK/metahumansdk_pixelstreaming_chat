#include "AssetFactories/SoundWaveFactory.h"
#include "Audio.h"
#include "Serialization/BulkData.h"
#include "Sound/SoundWave.h"
#include "AudioCompressionSettings.h"
#include "Interfaces/IAudioFormat.h"
#include "VorbisAudioInfo.h"

#if WITH_OGGVORBIS
    #pragma pack(push, 8)
    #include "vorbis/vorbisenc.h"
    #include "vorbis/vorbisfile.h"
    #pragma pack(pop)
#endif

static_assert(WITH_OGGVORBIS, "No point in compiling the OGG compressor if we don't have Vorbis.");

// Vorbis encoded sound is about 15% better quality than XMA - adjust the quality setting to get consistent cross platform sound quality
#define VORBIS_QUALITY_MODIFIER        0.85

#define SAMPLES_TO_READ        1024
#define SAMPLE_SIZE            ( ( uint32 )sizeof( short ) )

static FName NAME_OGG(TEXT("OGG"));

#if WITH_EDITOR
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistryModule.h"
#endif

#include "MetahumanSDKSettings.h"
#include "Helpers/SoundUtilityLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKSoundWaveFactory, Log, All);


bool FSoundWaveFactory::CookRawAudio(const TArray<uint8>& SrcBuffer, FSoundQualityInfo& QualityInfo, TArray<uint8>& OutData)
{
#if WITH_OGGVORBIS
    {

        short                ReadBuffer[SAMPLES_TO_READ * SAMPLE_SIZE * 2];

        ogg_stream_state    os;        // take physical pages, weld into a logical stream of packets 
        ogg_page            og;        // one ogg bitstream page.  Vorbis packets are inside
        ogg_packet            op;        // one raw packet of data for decode
        vorbis_info            vi;        // struct that stores all the static vorbis bitstream settings
        vorbis_comment        vc;        // struct that stores all the user comments
        vorbis_dsp_state    vd;        // central working state for the packet->PCM decoder
        vorbis_block        vb;        // local working space for packet->PCM decode
        uint32                i;
        bool                eos;

        // Create a buffer to store compressed data
        OutData.Empty();
        FMemoryWriter CompressedData(OutData);
        uint32 BufferOffset = 0;

        float CompressionQuality = (float)(QualityInfo.Quality * VORBIS_QUALITY_MODIFIER) / 100.0f;
        CompressionQuality = FMath::Clamp(CompressionQuality, -0.1f, 1.0f);

        vorbis_info_init(&vi);

        if (vorbis_encode_init_vbr(&vi, QualityInfo.NumChannels, QualityInfo.SampleRate, CompressionQuality))
        {
            return false;
        }

        // add a comment
        vorbis_comment_init(&vc);
        vorbis_comment_add_tag(&vc, "ENCODER", "UnrealEngine4");

        // set up the analysis state and auxiliary encoding storage
        vorbis_analysis_init(&vd, &vi);
        vorbis_block_init(&vd, &vb);

        // set up our packet->stream encoder
        ogg_stream_init(&os, 0);

        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header);
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        // This ensures the actual audio data will start on a new page, as per spec
        while (true)
        {
            int result = ogg_stream_flush(&os, &og);
            if (result == 0)
            {
                break;
            }

            CompressedData.Serialize(og.header, og.header_len);
            CompressedData.Serialize(og.body, og.body_len);
        }

        eos = false;
        while (!eos)
        {
            // Read samples
            uint32 BytesToRead = FMath::Min(SAMPLES_TO_READ * QualityInfo.NumChannels * SAMPLE_SIZE, QualityInfo.SampleDataSize - BufferOffset);
            FMemory::Memcpy(ReadBuffer, SrcBuffer.GetData() + BufferOffset, BytesToRead);
            BufferOffset += BytesToRead;

            if (BytesToRead == 0)
            {
                // end of file
                vorbis_analysis_wrote(&vd, 0);
            }
            else
            {
                // expose the buffer to submit data
                float** buffer = vorbis_analysis_buffer(&vd, SAMPLES_TO_READ);

                if (QualityInfo.NumChannels == 1)
                {
                    for (i = 0; i < BytesToRead / SAMPLE_SIZE; i++)
                    {
                        buffer[0][i] = (ReadBuffer[i]) / 32768.0f;
                    }
                }
                else
                {
                    for (i = 0; i < BytesToRead / (SAMPLE_SIZE * 2); i++)
                    {
                        buffer[0][i] = (ReadBuffer[i * 2]) / 32768.0f;
                        buffer[1][i] = (ReadBuffer[i * 2 + 1]) / 32768.0f;
                    }
                }

                // tell the library how many samples we actually submitted
                vorbis_analysis_wrote(&vd, i);
            }

            // vorbis does some data preanalysis, then divvies up blocks for more involved (potentially parallel) processing.
            while (vorbis_analysis_blockout(&vd, &vb) == 1)
            {
                // analysis, assume we want to use bitrate management
                vorbis_analysis(&vb, NULL);
                vorbis_bitrate_addblock(&vb);

                while (vorbis_bitrate_flushpacket(&vd, &op))
                {
                    // weld the packet into the bitstream
                    ogg_stream_packetin(&os, &op);

                    // write out pages (if any)
                    while (!eos)
                    {
                        int result = ogg_stream_pageout(&os, &og);
                        if (result == 0)
                        {
                            break;
                        }
                        CompressedData.Serialize(og.header, og.header_len);
                        CompressedData.Serialize(og.body, og.body_len);

                        // this could be set above, but for illustrative purposes, I do    it here (to show that vorbis does know where the stream ends)
                        if (ogg_page_eos(&og))
                        {
                            eos = true;
                        }
                    }
                }
            }
        }

        // clean up and exit.  vorbis_info_clear() must be called last
        ogg_stream_clear(&os);
        vorbis_block_clear(&vb);
        vorbis_dsp_clear(&vd);
        vorbis_comment_clear(&vc);
        vorbis_info_clear(&vi);
        // ogg_page and ogg_packet structs always point to storage in libvorbis.  They're never freed or manipulated directly
    }
    return OutData.Num() > 0;
#else
    return false;
#endif        // WITH_OGGVOBVIS
}


// duplicate of USoundWave::GetPlatformSpecificFormat
FName GetPlatformSpecificFormat(FName Format, const FPlatformAudioCookOverrides* CompressionOverrides)
{
    // Platforms that require compression overrides get concatenated formats.
#if WITH_EDITOR
    FName PlatformSpecificFormat;
    if (CompressionOverrides)
    {
        FString HashedString = *Format.ToString();
        FPlatformAudioCookOverrides::GetHashSuffix(CompressionOverrides, HashedString);
        PlatformSpecificFormat = *HashedString;
    }
    else
    {
        PlatformSpecificFormat = Format;
    }
#else
    if (CompressionOverrides == nullptr)
    {
        CompressionOverrides = USoundWave::GetPlatformCompressionOverridesForCurrentPlatform();
    }

    // Cache the concatenated hash:
    static FName PlatformSpecificFormat;
    static FName CachedFormat;
    if (!Format.IsEqual(CachedFormat))
    {
        if (CompressionOverrides)
        {
            FString HashedString = *Format.ToString();
            FPlatformAudioCookOverrides::GetHashSuffix(CompressionOverrides, HashedString);
            PlatformSpecificFormat = *HashedString;
        }
        else
        {
            PlatformSpecificFormat = Format;
        }

        CachedFormat = Format;
    }

#endif // WITH_EDITOR

    return PlatformSpecificFormat;
}

void CompressSoundWave(const FWaveModInfo& WaveInfo, USoundWave* SoundWave, FString& Error)
{
    FSoundQualityInfo QualityInfo = { 0 };
    const FPlatformAudioCookOverrides* CompressionOverrides = USoundWave::GetPlatformCompressionOverridesForCurrentPlatform();
    {
        if (CompressionOverrides)
        {
#if ENGINE_MAJOR_VERSION == 5
            float ModifiedCompressionQuality = (float)SoundWave->GetCompressionQuality() * CompressionOverrides->CompressionQualityModifier;
#else
            float ModifiedCompressionQuality = (float)SoundWave->CompressionQuality * CompressionOverrides->CompressionQualityModifier;
#endif

            if (ModifiedCompressionQuality >= 1.0f)
            {
                QualityInfo.Quality = FMath::FloorToInt(ModifiedCompressionQuality);
            }
            else
            {
#if ENGINE_MAJOR_VERSION == 5
                QualityInfo.Quality = SoundWave->GetCompressionQuality();
#else
                QualityInfo.Quality = SoundWave->CompressionQuality;
#endif
            }
        }
        else
        {
#if ENGINE_MAJOR_VERSION == 5
            QualityInfo.Quality = SoundWave->GetCompressionQuality();
#else
            QualityInfo.Quality = SoundWave->CompressionQuality;
#endif
        }

        QualityInfo.NumChannels = *WaveInfo.pChannels;
        QualityInfo.SampleRate = *WaveInfo.pSamplesPerSec;
        QualityInfo.SampleDataSize = WaveInfo.SampleDataSize;
        QualityInfo.bStreaming = false;
        QualityInfo.DebugName = SoundWave->GetFullName();
    }

    TArray<uint8> Input;
    Input.AddUninitialized(WaveInfo.SampleDataSize);
    FMemory::Memcpy(Input.GetData(), WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

    TArray<uint8> CookedOggData;
    if ( !FSoundWaveFactory::CookRawAudio(Input, QualityInfo, CookedOggData) )
    {
        Error = TEXT("Corrupt .wav data!");
        return;
    }
    
    if( !FSoundUtilityLibrary::ApplyCompressedData(Audio::NAME_OGG, CookedOggData, SoundWave) )
    {
        Error = TEXT("Can't Apply Compressed Data to the SoundWave!");
        return;
    }
    
    /*
#if ENGINE_MAJOR_VERSION == 5
    FByteBulkData* CompressedOggData = SoundWave->GetCompressedData(GetPlatformSpecificFormat(NAME_OGG, CompressionOverrides));
    if (CompressedOggData)
    {
        CompressedOggData->Lock(LOCK_READ_WRITE);
        FMemory::Memcpy(CompressedOggData->Realloc(CookedOggData.Num()), CookedOggData.GetData(), CookedOggData.Num());
        CompressedOggData->Unlock();
    }
#else
    FByteBulkData* CompressedOggData = &SoundWave->CompressedFormatData.GetFormat(GetPlatformSpecificFormat(NAME_OGG, CompressionOverrides));
    {
        CompressedOggData->Lock(LOCK_READ_WRITE);
        FMemory::Memcpy(CompressedOggData->Realloc(OutData.Num()), OutData.GetData(), OutData.Num());
        CompressedOggData->Unlock();
    }
#endif

    {
        SoundWave->SetSampleRate(QualityInfo.SampleRate);

        if (SoundWave->NumChannels != *WaveInfo.pChannels)
        {
            UE_LOG(LogTemp, Warning, TEXT("Updated SoundWave->NumChannels from %d to %u while cooking %s."), SoundWave->NumChannels, *WaveInfo.pChannels, *SoundWave->GetFullName());
            SoundWave->NumChannels = *WaveInfo.pChannels;
        }
        if (SoundWave->RawPCMDataSize != Input.Num())
        {
            UE_LOG(LogTemp, Display, TEXT("Updated SoundWave->RawPCMDataSize from %d to %d while cooking %s."), SoundWave->RawPCMDataSize, Input.Num(), *SoundWave->GetFullName());
            SoundWave->RawPCMDataSize = Input.Num();
        }

        // Update duration if it has changed > 1ms
        float NewDuration = (float)SoundWave->RawPCMDataSize / (QualityInfo.SampleRate * sizeof(int16) * SoundWave->NumChannels);
        if (!FMath::IsNearlyEqual(SoundWave->Duration, NewDuration, 0.001f))
        {
            UE_LOG(LogTemp, Display, TEXT("Updated SoundWave->Duration from %f to %f while cooking %s."), SoundWave->Duration, NewDuration, *SoundWave->GetFullName());
            SoundWave->Duration = NewDuration;
        }
    }
    
    FByteBulkData* CompressedData = SoundWave->GetCompressedData(NAME_OGG, CompressionOverrides);
    {
        CompressedData->Lock(LOCK_READ_WRITE);
        FMemory::Memcpy(CompressedData->Realloc(CookedOggData.Num()), CookedOggData.GetData(), CookedOggData.Num());
        CompressedData->Unlock();
    }
    */
}

USoundWave* FSoundWaveFactory::CreateSoundWave(const TArray<uint8>& RawData, FString& Error, UObject* InParent, const FString& SoundWaveName)
{
    QUICK_SCOPE_CYCLE_COUNTER(MetahumanSDK_CreateSoundWave);    
    const double StartTime = FPlatformTime::Seconds();

    FWaveModInfo WaveInfo;
    if (!WaveInfo.ReadWaveInfo(RawData.GetData(), RawData.Num(), &Error))
    {
        UE_LOG(LogMetahumanSDKSoundWaveFactory, Log, TEXT("Can't Read Wave Info"));
        return nullptr;
    }

    const int32 ChannelCount = (int32)*WaveInfo.pChannels;
    check(ChannelCount > 0);

    const int32 SizeOfSample = (*WaveInfo.pBitsPerSample) / 8;
    const int32 NumSamples = WaveInfo.SampleDataSize / SizeOfSample;    
    const int32 NumFrames = NumSamples / ChannelCount;

    UPackage* SoundWavePackage = Cast<UPackage>(InParent);

#if WITH_EDITOR
    if (MetahumanSDKDevSettings::bCreateAssetsInEditor)
    {
        FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

        FString DefaultSuffix;
        FString AssetName;
        FString PackageName;
        AssetToolsModule.Get().CreateUniqueAssetName(TEXT("/MetahumanSDK/DebugAssets/TTS/SoundWave"), DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);

        SoundWavePackage = CreatePackage(*PackageName);
        SoundWavePackage->SetFlags(RF_Standalone | RF_Public);
        SoundWavePackage->FullyLoad();
    }
#endif

    USoundWave* SoundWave = NewObject<USoundWave>(IsValid(SoundWavePackage) ? SoundWavePackage : GetTransientPackage(), *SoundWaveName, RF_Public | RF_Standalone);
    {    
#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
        SoundWave->RawData.UpdatePayload(FSharedBuffer::Clone(RawData.GetData(), RawData.Num()));
#else   // < 5.1     
        SoundWave->RawData.Lock(LOCK_READ_WRITE);
        void* LockedData = SoundWave->RawData.Realloc(RawData.Num());
        FMemory::Memcpy(LockedData, RawData.GetData(), RawData.Num());
        SoundWave->RawData.Unlock();
#endif  // >= 5.1               
#endif  // Editor 
        SoundWave->RawPCMData = (uint8*)FMemory::Malloc(WaveInfo.SampleDataSize);
        FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);
        SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;

        // Set Sound Wave Info
        SoundWave->Duration = (float)NumFrames / *WaveInfo.pSamplesPerSec;
        SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
        SoundWave->NumChannels = ChannelCount;
        SoundWave->TotalSamples = *WaveInfo.pSamplesPerSec * SoundWave->Duration;

        CompressSoundWave(WaveInfo, SoundWave, Error);

#if WITH_EDITOR
        if (MetahumanSDKDevSettings::bCreateAssetsInEditor)
        {
            SoundWavePackage->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(SoundWave);
        }
//         SoundWave->SetRedrawThumbnail(true);
#endif
//         SoundWave->InvalidateCompressedData(false, false);
//         SoundWave->PostLoad();
     }

    const double EndTime = FPlatformTime::Seconds();
    UE_LOG(LogMetahumanSDKSoundWaveFactory, Log, TEXT("Creating SoundWave took %f seconds to complete"), EndTime - StartTime);

    return SoundWave;
}