#include "Helpers/SoundUtilityLibrary.h"
#include "SoundWaveFactory.h"

#include "AudioCompressionSettingsUtils.h"
#include "AudioDecompress.h"
#include "BinkAudioDecoder/Module/Public/BinkAudioInfo.h"
#include "AudioDevice.h"
#include "SoundWaveDecoder.h"
#include "AudioMixer/Public/AudioMixerDevice.h"
#include "AudioMixer/Public/SoundWaveDecoder.h"
#include "Interfaces/IAudioFormat.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "Interfaces/ITargetPlatform.h"
#include "Async/Async.h"

#include "ThirdParty/dr_wav.h"


DEFINE_LOG_CATEGORY_STATIC(LogSoundUtilityLibrary, Log, All);

bool FSoundUtilityLibrary::GetSoundWaveData(USoundWave* InSoundWave, TArray<uint8>& OutAudioData)
{
	if( !IsValid(InSoundWave) )
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Input Sound Wave isn't valid!"), *FString(__FUNCTION__));
		return false;
	}

	// in case we get Procedural Sound Wave, prepare it for further reading
	if (USoundWaveProcedural* ProceduralSoundWave = Cast<USoundWaveProcedural>(InSoundWave))
	{
		if (!PrepareProceduralSoundWave(ProceduralSoundWave))
		{
			UE_LOG(LogSoundUtilityLibrary, Warning, TEXT("%s -- Can't prepare received Procedural Sound Wave!"), *FString(__FUNCTION__));
		}
	}
	
	// Try to get Cooked Platform Data first 
	if( GetCookedPlatformData(InSoundWave, (Audio::NAME_OGG).ToString(),OutAudioData) )
	{
		return true;	
	}
	
#if WITH_EDITOR	
	return FSoundUtilityLibrary::GetEditorSoundData(InSoundWave, OutAudioData);
#else
	return  FSoundUtilityLibrary::GetPackagedSoundData(InSoundWave, OutAudioData);	
#endif
}

bool FSoundUtilityLibrary::ApplyCompressedData(const FName AudioFormatName, const TArray<uint8>& CompressedData, USoundWave* InSoundWave)
{	
	FStreamedAudioPlatformData *PlatformData = new FStreamedAudioPlatformData();
	
	// Build first chunk (we don't support streaming input on the server yet)
	FStreamedAudioChunk* NewChunk = new FStreamedAudioChunk();	
	PlatformData->Chunks.Add(NewChunk);
	NewChunk->DataSize = CompressedData.Num();
	NewChunk->AudioDataSize = NewChunk->DataSize;
	
	if (NewChunk->BulkData.IsLocked())
	{
		UE_LOG(LogSoundUtilityLibrary, Warning, TEXT("NewChunk is Locked! SoundWave: %s "), *InSoundWave->GetFullName());
		return  false;
	}	
	NewChunk->BulkData.Lock(LOCK_READ_WRITE);
	void* NewChunkData = NewChunk->BulkData.Realloc(CompressedData.Num());
	FMemory::Memcpy(NewChunkData, CompressedData.GetData(), CompressedData.Num());
	NewChunk->BulkData.Unlock();
	
	// Append chunk to Sound Wave	
	InSoundWave->CookedPlatformData.Add( AudioFormatName.ToString() , PlatformData );
	
	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("Apply Compressed data: %d"), NewChunk->DataSize);
	return true;
}

bool FSoundUtilityLibrary::PrepareProceduralSoundWave(USoundWaveProcedural* InProceduralSoundWave)
{	
 	FSoundDescription SoundDescription;
	SoundDescription.NumChannels = InProceduralSoundWave->NumChannels;
	SoundDescription.SampleRate = InProceduralSoundWave->GetSampleRateForCurrentPlatform();
	SoundDescription.bBitPerSample32 = InProceduralSoundWave->GetGeneratedPCMDataFormat() == Audio::EAudioMixerStreamDataFormat::Float;
	
	// Generate PCM data
	const int32 MaxSamples = MONO_PCM_BUFFER_SAMPLES * SoundDescription.NumChannels;	
	TArray<uint8> PCMAudioData;
	int32 NumBytesWritten = 0;
	do
	{
		TArray<uint8> GeneratedData;
		NumBytesWritten = InProceduralSoundWave->OnGeneratePCMAudio(GeneratedData, MaxSamples);
		PCMAudioData.Append(GeneratedData);		
	}
	while (NumBytesWritten !=0);
	
	if (PCMAudioData.Num() == 0)
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Can't Generate PCM Audio!"), *FString(__FUNCTION__));
		return false;
	}	
	SoundDescription.SampleDataSize = PCMAudioData.Num(); 

	
	TArray<uint8> WavData;	
	if (!EncodePcmToWav(PCMAudioData.GetData(), SoundDescription, WavData))
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Can't Encode to Wav!"), *FString(__FUNCTION__));
		return false;
	}	

#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	InProceduralSoundWave->RawData.UpdatePayload(FSharedBuffer::Clone(WavData.GetData(), WavData.Num()));
#else   // < 5.1     
	InProceduralSoundWave->RawData.Lock(LOCK_READ_WRITE);
	void* LockedData = InProceduralSoundWave->RawData.Realloc(WavData.Num());
	FMemory::Memcpy(LockedData, WavData.GetData(), WavData.Num());
	InProceduralSoundWave->RawData.Unlock();
#endif  // >= 5.1               
#else // Packaged
	InProceduralSoundWave->RawPCMData = (uint8*)FMemory::Malloc(PCMAudioData.Num());
	FMemory::Memcpy(InProceduralSoundWave->RawPCMData, PCMAudioData.GetData(), PCMAudioData.Num());
	InProceduralSoundWave->RawPCMDataSize = PCMAudioData.Num();
#endif  // Editor

	
	
	// Compress Audio

	// test encoding to int16
	// TArray<uint8> int16_PCMAudioData;
	// int16_PCMAudioData.SetNumUninitialized(PCMAudioData.Num());
	// TArray<uint8> int32_PCMAudioData = PCMAudioData;
	// for (int32 i = 0; i < int32_PCMAudioData.Num(); i++)
	// {
	// 	int16_PCMAudioData[i] = int32_PCMAudioData[i] * 32767.0f;
	// }
	// PCMAudioData = int16_PCMAudioData;
	
	// TArray<uint8> CompressedData;
	// FSoundQualityInfo SoundQualityInfo;
	// SoundQualityInfo.NumChannels = SoundDescription.NumChannels;
	// SoundQualityInfo.SampleRate = SoundDescription.SampleRate;
	// SoundQualityInfo.SampleDataSize = PCMAudioData.Num();
	// SoundQualityInfo.bStreaming = false;
	// SoundQualityInfo.Quality = InProceduralSoundWave->GetCompressionQuality();
	// if ( !FSoundWaveFactory::CookRawAudio(PCMAudioData, SoundQualityInfo, CompressedData) )
	// {
	// 	UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Can't Compress Audio!"), *FString(__FUNCTION__));
	// 	return false;
	// }
	//  
	// if( !FSoundUtilityLibrary::ApplyCompressedData(Audio::NAME_OGG, CompressedData, InProceduralSoundWave) )
	// {
	// 	UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Can't Apply Compressed Data!"), *FString(__FUNCTION__));
	// 	return false;
	// }
	
	return true;
}

bool FSoundUtilityLibrary::GetCookedPlatformData(const USoundWave* InSoundWave, const FString& AudioFormat, TArray<uint8>& OutData)
{
	const TSortedMap<FString, FStreamedAudioPlatformData*> CookedPlatformData_Local = InSoundWave->CookedPlatformData;
	if(CookedPlatformData_Local.IsEmpty())
	{		
		return false;
	}
	
	const FStreamedAudioPlatformData* PlatformData = *(CookedPlatformData_Local.Find(AudioFormat));	
	if (!PlatformData)
	{
		UE_LOG(LogSoundUtilityLibrary, Warning, TEXT("%s -- Can't find Audio Format: %s"), *FString(__FUNCTION__), *AudioFormat);
		return false;
	}		

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	TIndirectArray<struct FStreamedAudioChunk>& Chunks = PlatformData->GetChunks();
#else	// < 5.1
	TIndirectArray<struct FStreamedAudioChunk> Chunks = PlatformData->Chunks;
#endif	// >= 5.1
	
	if (Chunks.Num() == 0)
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Chunks are empty!"), *FString(__FUNCTION__));
		return  false;
	}
	
	// Read the first chunk (we don't support streaming input on the server yet)	
	OutData.Empty();
	OutData.AddUninitialized(Chunks[0].DataSize);
	void* DataDestPtr = OutData.GetData();
	Chunks[0].BulkData.GetCopy(&DataDestPtr, false);
	
	if (OutData.Num() == 0)
	{	
		return false;
	}	

	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- Out Data size: %d"), *FString(__FUNCTION__), OutData.Num());
	return  true;
}

#if WITH_EDITOR	
bool FSoundUtilityLibrary::GetEditorSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	auto RawBulkData = InSoundWave->RawData.GetPayload().Get();
	if(!RawBulkData)
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- RawBulkData isn't Valid!"), *FString(__FUNCTION__));
		return false;
	}
	
	OutData.SetNumUninitialized(RawBulkData.GetSize());
	FMemory::Memcpy(OutData.GetData(), RawBulkData.GetData(), RawBulkData.GetSize());
#else   // < 5.1     
	ensure(!InSoundWave->RawData.IsLocked());
	void* RawBulkData = InSoundWave->RawData.Lock(LOCK_READ_ONLY);
	if(!RawBulkData)
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- RawBulkData isn't Valid!"), *FString(__FUNCTION__));
		InSoundWave->RawData.Unlock();
		return false;
	}
	OutData.SetNumUninitialized(InSoundWave->RawData.GetBulkDataSize());
	FMemory::Memcpy(OutData.GetData(), RawBulkData, InSoundWave->RawData.GetBulkDataSize());
	InSoundWave->RawData.Unlock();
#endif  // >= 5.1	

	// Check Audio Data
	if (OutData.Num() == 0)
	{		
		return false;
	}	

	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- Out Data size: %d"), *FString(__FUNCTION__), OutData.Num());
	return  true;
}

#else
bool FSoundUtilityLibrary::GetPackagedSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData)
{
	// Raw Pcm Data
	if (InSoundWave->RawPCMDataSize != 0)
	{
		UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- Raw PCM Data Size: %d"), *FString(__FUNCTION__), InSoundWave->RawPCMDataSize);
		
		const USoundWaveProcedural* ProceduralSoundWave = Cast<USoundWaveProcedural>(InSoundWave);
		if(ProceduralSoundWave)
		{
			FSoundDescription SoundDescription;
			SoundDescription.NumChannels = ProceduralSoundWave->NumChannels;
			SoundDescription.SampleRate = ProceduralSoundWave->GetSampleRateForCurrentPlatform();
			SoundDescription.bBitPerSample32 = ProceduralSoundWave->GetGeneratedPCMDataFormat() == Audio::EAudioMixerStreamDataFormat::Float;
			SoundDescription.SampleDataSize = ProceduralSoundWave->RawPCMDataSize;

			return FSoundUtilityLibrary::EncodePcmToWav(InSoundWave->RawPCMData, SoundDescription, OutData);	
		}

		const FSoundWaveProxyPtr SoundWaveProxyPtr = InSoundWave->CreateSoundWaveProxy();
		FSoundQualityInfo SoundQualityInfo;
		SoundQualityInfo.NumChannels = SoundWaveProxyPtr->GetNumChannels();
		SoundQualityInfo.SampleRate = SoundWaveProxyPtr->GetSampleRate();
		SoundQualityInfo.SampleDataSize = InSoundWave->RawPCMDataSize;
	
		return FSoundUtilityLibrary::EncodePcmToWav(InSoundWave->RawPCMData, SoundQualityInfo, OutData);	
	}

	// Chunked Data
	TArray<uint8> ChunkedSoundData;
	if ( !FSoundUtilityLibrary::GetChunkedSoundData(InSoundWave, ChunkedSoundData) )
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Can't get Chunked Data!"), *FString(__FUNCTION__));
		return false;
	}
		
	if(IsNeedDecompress(InSoundWave))
	{
		FSoundQualityInfo SoundQualityInfo;
		TArray<uint8> DecompressedSoundData;

		if ( !FSoundUtilityLibrary::DecompressSoundData(ChunkedSoundData, SoundQualityInfo, DecompressedSoundData) )
		{
			return false;
		}
	
		return FSoundUtilityLibrary::EncodePcmToWav(DecompressedSoundData, SoundQualityInfo, OutData);	
	}
	else
	{
		OutData = MoveTemp(ChunkedSoundData);
		UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- Out Packaged Sound Data size: %d"), *FString(__FUNCTION__), OutData.Num());
		return true;
	}	
}

bool FSoundUtilityLibrary::GetCompressedSoundFormatName(const USoundWave* InSoundWave, FName& CompressedSoundFormat)
{
	// Check the supported Audio Formats
	const FAudioDevice* LocalAudioDevice = GEngine->GetMainAudioDeviceRaw();
	if (!LocalAudioDevice)
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("Local Audio Device is Null!"));
		return false;
	}

	const FName RuntimeFormat = LocalAudioDevice->GetRuntimeFormat(InSoundWave);
	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("Runtime Format: %s"), *RuntimeFormat.ToString());
	if (RuntimeFormat != FName("OGG"))
	{
		UE_LOG(LogSoundUtilityLibrary, Warning, TEXT("Runtime Format isn't compressed OGG!"));
	}
			
	// Check Compressed Data with Runtime Format	
	if (const bool bSoundHasFormat = InSoundWave->HasCompressedData(RuntimeFormat); !bSoundHasFormat)
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("Sound hasn't format: %s"), *RuntimeFormat.ToString());
		return false;
	}

	// Get Format Hash String in the Compressed Data
	const FPlatformAudioCookOverrides* CompressionOverrides = FPlatformCompressionUtilities::GetCookOverrides();
	FString FormatHashedString = *RuntimeFormat.ToString();
	FPlatformAudioCookOverrides::GetHashSuffix(CompressionOverrides, FormatHashedString);
	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("Format Hashed string: %s"), *FormatHashedString);

	CompressedSoundFormat = FName(FormatHashedString);	
	
	return true;
}

bool FSoundUtilityLibrary::GetFormatSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData)
{
	FName CompressedSoundFormat;	
	if(!FSoundUtilityLibrary::GetCompressedSoundFormatName(InSoundWave, CompressedSoundFormat))
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("Couldn't get Compressed Sound Format!"));
		return false;
	}	
			
	// Fill Audio From Compressed Data
	FByteBulkData FormatData = *(InSoundWave->GetCompressedData(CompressedSoundFormat));
	const void* RawBulkData = FormatData.Lock(LOCK_READ_ONLY);
	OutData.SetNumUninitialized(FormatData.GetBulkDataSize());
	FMemory::Memcpy(OutData.GetData(), RawBulkData, FormatData.GetBulkDataSize());
	FormatData.Unlock();
	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("Audio data length: %d"), OutData.Num());

	// Check Audio Data
	if (OutData.Num() == 0)
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("Couldn't fill Audio Data!"));
		return false;
	}	
	
	return true;
}

bool FSoundUtilityLibrary::GetChunkedSoundData(USoundWave* InSoundWave, TArray<uint8>& OutData)
{	
	const FSoundWaveProxyPtr SoundWaveProxyPtrProxy = InSoundWave->CreateSoundWaveProxy();
	for (uint32 CurrentChunk = 0; CurrentChunk < InSoundWave->GetNumChunks(); CurrentChunk++)
	{
		if (CurrentChunk == 0)
		{
			TArrayView<const uint8> ZerothChunk = InSoundWave->GetZerothChunk(true);
			
			OutData.Append(ZerothChunk.GetData(), ZerothChunk.Num());
		}
		else
		{
			FAudioChunkHandle AudioChunkHandle = IStreamingManager::Get().GetAudioStreamingManager().GetLoadedChunk(SoundWaveProxyPtrProxy, CurrentChunk, true, false);
			OutData.Append(AudioChunkHandle.GetData(), AudioChunkHandle.Num());
		}				
	}
	
	// Check Audio Data	
	if (OutData.Num() == 0)
	{
		return false;
	}
	
	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- Audio Data Size from chunks: %d"), *FString(__FUNCTION__), OutData.Num());	
	return true;
}

bool FSoundUtilityLibrary::IsNeedDecompress(const USoundWave* InSoundWave)
{
	if(GetSoundRuntimeFormat(InSoundWave) == "BINKA")
	{
		return  true;
	}
	
	return  false;
}

FName FSoundUtilityLibrary::GetSoundRuntimeFormat(const USoundWave* InSoundWave)
{
	FName SoundRuntimeFormat = "";
	
	if(FAudioDeviceManager* AudioDeviceManager = GEngine->GetAudioDeviceManager())
	{
		if (FAudioDevice* AudioDevice = AudioDeviceManager->GetActiveAudioDevice().GetAudioDevice())
		{
			Audio::FMixerDevice* MixerDevice = static_cast<Audio::FMixerDevice*>(AudioDevice);
			if(Audio::IAudioMixerPlatformInterface* AudioMixerPlatform = MixerDevice->GetAudioMixerPlatform())
			{				 
				SoundRuntimeFormat = AudioMixerPlatform->GetRuntimeFormat(InSoundWave);
				UE_LOG(LogSoundUtilityLibrary, Warning, TEXT("Runtime format : %s"), *(AudioMixerPlatform->GetRuntimeFormat(InSoundWave).ToString()));	
			}			
		}
	}
	
	return SoundRuntimeFormat;
}

bool FSoundUtilityLibrary::DecompressSoundData(const TArray<uint8>& InData, FSoundQualityInfo& OutSoundQuality, TArray<uint8>& OutData)
{
	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- Start decompressing"), *FString(__FUNCTION__));

	const TUniquePtr<IStreamedCompressedInfo> Decoder = MakeUnique<FBinkAudioInfo>();
		
	FSoundQualityInfo SoundQualityInfo;	
	Decoder->ParseHeader(InData.GetData(), InData.Num(), &SoundQualityInfo);
	Decoder->CreateDecoder();

	const uint32 BufferSize = SoundQualityInfo.SampleDataSize;
	OutData.SetNumUninitialized(BufferSize);
	OutSoundQuality = SoundQualityInfo;
	
	Decoder->ReadCompressedData(OutData.GetData(), false, BufferSize);	
	
	if (OutData.Num() == 0)
	{
		return false;
	}	

	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- Decompressed Sound Data Size: %d"), *FString(__FUNCTION__), OutData.Num());
	return true;
}


#endif

bool FSoundUtilityLibrary::EncodePcmToWav(const TArray<uint8>& PcmData, const FSoundQualityInfo SoundQuality, TArray<uint8>& WavData)
{
	return EncodePcmToWav(PcmData.GetData(), SoundQuality, WavData);
}

bool FSoundUtilityLibrary::EncodePcmToWav(const void* PcmDataPtr, const FSoundQualityInfo SoundQuality, TArray<uint8>& WavData)
{
	const FSoundDescription SoundDescription( SoundQuality.NumChannels, SoundQuality.SampleRate,  SoundQuality.SampleDataSize );
	
	return EncodePcmToWav(PcmDataPtr, SoundDescription, WavData);
}

bool FSoundUtilityLibrary::EncodePcmToWav(const void* PcmDataPtr, const FSoundDescription SoundDescription, TArray<uint8>& WavData)
{
	drwav WavEncoder;
	
	drwav_data_format WavFormat;
	WavFormat.container = drwav_container_riff;	
	WavFormat.channels = SoundDescription.NumChannels;
	WavFormat.sampleRate = SoundDescription.SampleRate;

	if (SoundDescription.bBitPerSample32)
	{
		WavFormat.format = DR_WAVE_FORMAT_IEEE_FLOAT;	
		WavFormat.bitsPerSample = 32;	
	}
	else
	{
		WavFormat.format = DR_WAVE_FORMAT_PCM;	
		WavFormat.bitsPerSample = 16;	
	}	

	void* EncodedData = nullptr;
	size_t EncodedDataSize;
	if (!drwav_init_memory_write(&WavEncoder, &EncodedData, &EncodedDataSize, &WavFormat, nullptr))
	{
		UE_LOG(LogSoundUtilityLibrary, Error, TEXT("%s -- Couldn't Init Wav Encoder!"), *FString(__FUNCTION__));
		return false;
	}	
	
	drwav_write_raw(&WavEncoder, SoundDescription.SampleDataSize, PcmDataPtr);

	drwav_uninit(&WavEncoder);

	WavData.Empty();
	WavData.SetNumUninitialized(EncodedDataSize);
	FMemory::Memcpy(WavData.GetData(), EncodedData, EncodedDataSize);
		
	if (WavData.Num() == 0)
	{
		return false;
	}		

	UE_LOG(LogSoundUtilityLibrary, Display, TEXT("%s -- WAV Data Size: %d"), *FString(__FUNCTION__), static_cast<int>(EncodedDataSize));
	return true;		
}
