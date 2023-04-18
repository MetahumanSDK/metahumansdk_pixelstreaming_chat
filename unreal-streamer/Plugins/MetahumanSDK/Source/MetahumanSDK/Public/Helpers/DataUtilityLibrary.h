#pragma once

#include "CoreMinimal.h"


class METAHUMANSDK_API FDataUtilityLibrary
{
public:

	template<typename TFormatType>
	void CopyToBulkData(FByteBulkData& Out, const TArray<typename TFormatType::Type>& Data)
	{
		const uint32 DataSizeInByte = Data.Num() * sizeof(typename TFormatType::BulkType);

		// The buffer is then stored into bulk data
		Out.Lock(LOCK_READ_WRITE);
		void* BulkBuffer = Out.Realloc(DataSizeInByte);
		FMemory::Memcpy(BulkBuffer, Data.GetData(), DataSizeInByte);
		Out.Unlock();
	}
	
};


