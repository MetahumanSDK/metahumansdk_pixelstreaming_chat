// Fill out your copyright notice in the Description page of Project Settings.


#include "ATLStreamBuffer.h"

void UATLStreamBuffer::Clear()
{
	Buffer.Empty();
}

void UATLStreamBuffer::AddChunk(UAnimSequence* OutAnimation)
{
	Buffer.Add(OutAnimation);

	if (Buffer.Num() == WaitForChunksAmount)
	{
		if (OnFilled.IsBound())
		{
			OnFilled.Broadcast();
		}
	}
}

UAnimSequence* UATLStreamBuffer::GetChunk(int Index)
{
	return Index >= 0 && Index < Buffer.Num() ? Buffer[Index] : nullptr;
}