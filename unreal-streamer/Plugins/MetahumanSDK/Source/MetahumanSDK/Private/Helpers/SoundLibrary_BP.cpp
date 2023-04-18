// Fill out your copyright notice in the Description page of Project Settings.


#include "Helpers/SoundLibrary_BP.h"

#include "AssetFactories/SoundWaveFactory.h"

DEFINE_LOG_CATEGORY_STATIC(LogSoundLibrary_BP, Log, All);

bool USoundLibrary_BP::LoadSoundFromFile(const FString& FileName, USoundWave*& OutSound)
{
	TArray<uint8> LoadedFile;
	if (FFileHelper::LoadFileToArray(LoadedFile, *FileName))
	{
		FString Error;
		OutSound = FSoundWaveFactory::CreateSoundWave(LoadedFile, Error);

		if (Error.IsEmpty())
		{			
			return true;
		}

		UE_LOG(LogSoundLibrary_BP, Error, TEXT("%s -- Can't Create Sound Wave : %s"), *FString(__FUNCTION__), *Error);
	}
	
	return false;
}
