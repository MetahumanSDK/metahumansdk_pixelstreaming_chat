#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

// kubazip
#include "zip.h"


struct FLatentActionUtilities
{
    static void CompleteLatentAction(const FLatentActionInfo& LatentInfo)
    {
        if (LatentInfo.Linkage == INDEX_NONE)
        {
            return;
        }

        if (UObject* CallbackTarget = LatentInfo.CallbackTarget)
        {
            if (UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction))
            {
                CallbackTarget->ProcessEvent(ExecutionFunction, (void*)(&LatentInfo.Linkage));
            }
        }
    }
};


struct FJsonToStringUtilities
{
    static FString JsonObjectToString(TSharedPtr<FJsonObject> JsonObject)
    {
        FString ContentString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ContentString);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

        return ContentString;
    }
};


struct FMetahumanSDKZipUtilities
{
    static const int32 KUBAZIP_SUCCESS = 0;

    static bool ExtractArchive(const FString& ArchiveFilename, const FString& ExtractDirectory, FString& OutError)
    {
        const int ReturnCode = zip_extract(
            TCHAR_TO_ANSI(*ArchiveFilename),
            TCHAR_TO_ANSI(*ExtractDirectory),
            nullptr,
            nullptr
        );

        if (ReturnCode != KUBAZIP_SUCCESS)
        {
            OutError = FString::Printf(TEXT("Corrupt archive: %s"), *OutError);
        }

        return ReturnCode == KUBAZIP_SUCCESS;
    }
};