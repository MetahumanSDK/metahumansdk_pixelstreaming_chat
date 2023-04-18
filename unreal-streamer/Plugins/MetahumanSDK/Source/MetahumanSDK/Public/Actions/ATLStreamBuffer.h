// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetahumanSDKResponses.h"
#include "UObject/Object.h"
#include "ATLStreamBuffer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnATLStreamBufferFilled);
/**
 * 
 */
UCLASS(BlueprintType)
class METAHUMANSDK_API UATLStreamBuffer : public UObject
{
	GENERATED_BODY()

	public:

	UPROPERTY(BlueprintAssignable)
	FOnATLStreamBufferFilled OnFilled;

	int WaitForChunksAmount = 1;

	void Clear();

	void AddChunk(UAnimSequence* ATLResponse);

	UFUNCTION(BlueprintCallable, Category=Buffer)
	UAnimSequence* GetChunk(int Index);

	UPROPERTY(BlueprintReadOnly, Category=Buffer)
	TArray<UAnimSequence*> Buffer;

	UPROPERTY(BlueprintReadOnly, Category=Buffer)
	float ExpectedLength;
};
