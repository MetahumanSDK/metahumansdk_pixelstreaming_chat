#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

class USkeleton;

struct METAHUMANSDK_API FGenericResponse
{
    virtual ~FGenericResponse(){}
    virtual void ParseResponse(const TArray<uint8>&) {};
    virtual void ParseResponse(FString&&) {};
    virtual bool PassContentAsBytes() const 
    { 
        return true;
    }
    
    bool bSuccess = true;
    FString Error;
};

struct METAHUMANSDK_API FATLFrameInfo
{
    double Timesec;
    TArray<float> Blendshapes;
    TArray<FVector> Bones;
};

struct METAHUMANSDK_API FATLResponse : public FGenericResponse
{    
    virtual void ParseResponse(FString&& Content) override
    {
        // only for debugging
#if WITH_EDITOR
        FFileHelper::SaveStringToFile(Content, *(FPaths::ProjectSavedDir() / TEXT("MetahumanSDK") / TEXT("Temp") / TEXT("atl_temp.json")));
#endif

        TSharedRef<TJsonReader<>> Reader = FJsonStringReader::Create(MoveTemp(Content));

        TSharedPtr<FJsonObject> RootJson;
        if (FJsonSerializer::Deserialize(Reader, RootJson))
        {
            RootJson->TryGetStringField(TEXT("version"), Version);
            RootJson->TryGetStringField(TEXT("platform"), Platform);
            RootJson->TryGetNumberField(TEXT("fps"), FPS);
            RootJson->TryGetStringArrayField(TEXT("blendShapesNames"), BlendShapesNames);
            RootJson->TryGetStringArrayField(TEXT("bonesNames"), BonesNames);

            const TArray<TSharedPtr<FJsonValue>>* FramesArray;
            if (RootJson->TryGetArrayField(TEXT("frames"), FramesArray))
            {
                for (const TSharedPtr<FJsonValue>& FrameValue : *FramesArray)
                {
                    const TSharedPtr<FJsonObject>& FrameObject = FrameValue->AsObject();

                    FATLFrameInfo FrameInfo;
                    {
                        FrameObject->TryGetNumberField(TEXT("timesec"), FrameInfo.Timesec);

                        const TArray<TSharedPtr<FJsonValue>>* BlendshapeValues;
                        if (FrameObject->TryGetArrayField(TEXT("blendshapes"), BlendshapeValues))
                        {
                            for (const TSharedPtr<FJsonValue>& BlendshapeValue : *BlendshapeValues)
                            {
                                FrameInfo.Blendshapes.Add(BlendshapeValue->AsNumber());
                            }
                        }

                        const TArray<TSharedPtr<FJsonValue>>* BonesValues;
                        if (FrameObject->TryGetArrayField(TEXT("bones"), BonesValues))
                        {
                            for (const TSharedPtr<FJsonValue>& BoneValue : *BonesValues)
                            {
                                const TArray<TSharedPtr<FJsonValue>>* EulerValues;
                                BoneValue->AsObject()->TryGetArrayField(TEXT("eul"), EulerValues);

                                if (EulerValues->Num() > 2)
                                {
                                    FVector EulerValue((*EulerValues)[0]->AsNumber(), (*EulerValues)[1]->AsNumber(), (*EulerValues)[2]->AsNumber());
                                    FrameInfo.Bones.Add(EulerValue);
                                }
                            }
                        }
                    }

                    Frames.Add(FrameInfo);
                }
            }
        }

        if (!Validate())
        {
            bSuccess = false;
            Error = FString::Printf(TEXT("Invalid ATL response json: %s"), *Content);
        }
    }

#if WITH_EDITOR
    // json animation import case
    bool IsValidJsonString(const FString& Content)
    {
        bSuccess = true;
        
        TSharedRef<TJsonReader<>> Reader = FJsonStringReader::Create(Content);

        TSharedPtr<FJsonObject> RootJson;
        if (FJsonSerializer::Deserialize(Reader, RootJson))
        {
            bSuccess &= RootJson->HasField(TEXT("version"));
            bSuccess &= RootJson->HasField(TEXT("platform"));
            bSuccess &= RootJson->HasField(TEXT("fps"));
            bSuccess &= RootJson->HasField(TEXT("fps"));
            bSuccess &= RootJson->HasField(TEXT("frames"));
        }

        return bSuccess;
    }
#endif

    virtual bool PassContentAsBytes() const override
    {
        return false;
    }

    bool Validate() const
    {
        return Version.Len() > 0 && Platform.Len() > 0 && FPS > 0 && BlendShapesNames.Num() > 0 && Frames.Num() > 0 && Frames[0].Blendshapes.Num() == BlendShapesNames.Num();
    }

    FString Version;
    FString Platform;
    int32 FPS;
    TArray<FString> BlendShapesNames;
    TArray<FString> BonesNames;
    TArray<FATLFrameInfo> Frames;
};

struct METAHUMANSDK_API FTTSResponse : public FGenericResponse
{
    virtual void ParseResponse(const TArray<uint8>& InData) override
    {
        // only for debugging
        #if WITH_EDITOR
        FFileHelper::SaveArrayToFile(InData, *(FPaths::ProjectSavedDir() / TEXT("MetahumanSDK") / TEXT("Temp") / TEXT("tts_temp.wav")));
#endif
        
        Data = InData;
    }

    TArray<uint8> Data;
};

struct METAHUMANSDK_API FChatResponse : public FGenericResponse
{
    virtual void ParseResponse(FString&& Content) override
    {
        TSharedRef<TJsonReader<>> Reader = FJsonStringReader::Create(MoveTemp(Content));

        TSharedPtr<FJsonObject> RootJson;
        if (FJsonSerializer::Deserialize(Reader, RootJson))
        {
            RootJson->TryGetStringField(TEXT("reply"), Reply);
        }

        if (!Validate())
        {
            bSuccess = false;
            Error = FString::Printf(TEXT("Invalid Chat response json: %s"), *Content);
        }
    }

    virtual bool PassContentAsBytes() const override
    {
        return false;
    }

    bool Validate() const
    {
        return Reply.Len() > 0;
    }

    FString Reply = "";
};

struct METAHUMANSDK_API FComboResponse : public FGenericResponse
{
    virtual void ParseResponse(const TArray<uint8>& InData) override
    {
        Data = InData;
    }

    TArray<uint8> Data;
};