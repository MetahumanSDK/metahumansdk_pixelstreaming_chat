// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SoundLibrary_BP.generated.h"

/**
 * Sound library for using in the blueprint classes 
 */
UCLASS()
class METAHUMANSDK_API USoundLibrary_BP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	* Load sound wave from file (.wav) with proper preparation for ATL. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Sound Library for Blueprints")
	static bool LoadSoundFromFile(const FString& FileName, USoundWave*& OutSound);	
};
