#pragma once

#include "CoreMinimal.h"
#include "AudioDecompress.h"
#include "AudioDevice.h"
#include "Sound/SoundWave.h"
#include "Serialization/BulkData2.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "MetahumanSDKAPIInput.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Developer/TargetPlatform/Public/Interfaces/ITargetPlatform.h"
#include "Helpers/SoundUtilityLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKRequests, Log, All);

struct FGenericRequest
{
    virtual ~FGenericRequest() {}
    virtual void GenerateAttributesJson(TMap<FString, TSharedPtr<FJsonObject>>& Attributes) const = 0;
    virtual FString GetRequestType() const = 0;
    virtual FString GetAttributeName() const = 0;
};

struct FATLRequest : public FGenericRequest
{
    FATLRequest(const FMetahumanSDKATLInput& InATLInput, bool bGrabAudioData = true) : ATLInput(InATLInput)
	{
		if (bGrabAudioData)
		{
			bSuccessful = FSoundUtilityLibrary::GetSoundWaveData(InATLInput.Sound, AudioData);
		}
	}

    virtual void GenerateAttributesJson(TMap<FString, TSharedPtr<FJsonObject>>& Attributes) const override
    {
        TSharedPtr<FJsonObject> ATLJson = MakeShareable<FJsonObject>(new FJsonObject);
        {
            ATLJson->SetStringField(TEXT("platform"), TEXT("unreal"));
            ATLJson->SetStringField(TEXT("rigVersion"), ATLInput.RigVersion);
            ATLJson->SetStringField(TEXT("rigType"), ATLInput.MappingsInfo.ShouldSetUpForMetahumans() ? TEXT("metahuman") : TEXT("custom"));
            ATLJson->SetStringField(TEXT("engine"), ATLInput.Engine);
            ATLJson->SetStringField(TEXT("answerFormat"), TEXT("json-anim"));

            const FString ExplicitEmotionVal = UEnum::GetValueAsString(ATLInput.ExplicitEmotion).RightChop(FString(TEXT("EExplicitEmotion::")).Len());
            const FString ExplicitEmotion = ExplicitEmotionVal.Mid(1).ToLower();
            ATLJson->SetStringField(TEXT("explicitEmotion"), ExplicitEmotion);

            TSharedPtr<FJsonObject> EyeMovementParametersJson = MakeShareable<FJsonObject>(new FJsonObject);
            {
                EyeMovementParametersJson->SetStringField(TEXT("eyeMovementMode"), ATLInput.EyeMovementModeMocap ? TEXT("mocap") : TEXT("none"));
            }
            ATLJson->SetObjectField(TEXT("eyeMovementParameters"), EyeMovementParametersJson);

            TSharedPtr<FJsonObject> NeckMovementParametersJson = MakeShareable<FJsonObject>(new FJsonObject);
            {
                NeckMovementParametersJson->SetStringField(TEXT("neckMovementMode"), ATLInput.NeckMovementModeMocap ? TEXT("mocap") : TEXT("none"));
            }
            ATLJson->SetObjectField(TEXT("neckMovementParameters"), NeckMovementParametersJson);
        }
        
        Attributes.Add(GetAttributeName(), ATLJson);
    }

    virtual FString GetRequestType() const
    {
        return TEXT("audioToLipSync");
    }
    virtual FString GetAttributeName() const
    {
        return TEXT("atl");
    }

    FMetahumanSDKATLInput ATLInput;
    TArray<uint8> AudioData;
    bool bSuccessful = true;
};

struct FTTSRequest : public FGenericRequest
{
    FTTSRequest(const FMetahumanSDKTTSInput& InTTSInput) : TTSInput(InTTSInput) {}
   
    virtual void GenerateAttributesJson(TMap<FString, TSharedPtr<FJsonObject>>& Attributes) const override
    {
        TSharedPtr<FJsonObject> TTSJson = MakeShareable<FJsonObject>(new FJsonObject);
        {
            TTSJson->SetStringField(TEXT("ttsEngine"), TTSInput.TTSEngine);
            TTSJson->SetStringField(TEXT("voiceID"), TTSInput.VoiceID);
            TTSJson->SetStringField(TEXT("text"), TTSInput.Text);
            TTSJson->SetStringField(TEXT("textType"), TTSInput.TextType);
        }
        
        Attributes.Add(GetAttributeName(), TTSJson);
    }
    virtual FString GetRequestType() const
    {
        return TEXT("textToSpeech");
    }
    virtual FString GetAttributeName() const
    {
        return TEXT("tts");
    }

    FMetahumanSDKTTSInput TTSInput;
};

struct FChatRequest : public FGenericRequest
{
    FChatRequest(const FMetahumanSDKChatInput& InChatInput) : ChatInput(InChatInput) {}

    virtual void GenerateAttributesJson(TMap<FString, TSharedPtr<FJsonObject>>& Attributes) const override
    {
        TSharedPtr<FJsonObject> ChatJson = MakeShareable<FJsonObject>(new FJsonObject);
        {
            ChatJson->SetStringField(TEXT("engine"), ChatInput.Engine);
            ChatJson->SetNumberField(TEXT("chatID"), ChatID);

            const FString ChatLanguageVal = UEnum::GetValueAsString(ChatInput.Language).RightChop(FString(TEXT("EChatLanguage::")).Len());
            const FString ChatLanguage = ChatLanguageVal.Mid(2, 2) + TEXT("_") + ChatLanguageVal.Mid(4, 2);
            ChatJson->SetStringField(TEXT("languageCode"), ChatLanguage);

            TArray<TSharedPtr<FJsonValue>> ChatHistoryJsonArray;
            for (const FString& ChatMessage : ChatHistory)
            {
                ChatHistoryJsonArray.Add(MakeShareable(new FJsonValueString(ChatMessage)));
            }
            ChatJson->SetArrayField(TEXT("history"), ChatHistoryJsonArray);
        }

        Attributes.Add(GetAttributeName(), ChatJson);
    }

    virtual FString GetRequestType() const
    {
        return TEXT("chat");
    }
    virtual FString GetAttributeName() const
    {
        return TEXT("chat");
    }

    FMetahumanSDKChatInput ChatInput;
    int32 ChatID = -1;
    TArray<FString> ChatHistory;
};

struct FComboRequest : public FGenericRequest
{
    FComboRequest(EComboMode InComboMode, const FTTSRequest& InTTSRequest, const FATLRequest& InATLRequest, const FChatRequest& InChatRequest)
    : ComboMode(InComboMode), TTSRequest(InTTSRequest), ATLRequest(InATLRequest), ChatRequest(InChatRequest) {}

    virtual void GenerateAttributesJson(TMap<FString, TSharedPtr<FJsonObject>>& Attributes) const override
    {
        TSharedPtr<FJsonObject> AttributesJson = MakeShareable<FJsonObject>(new FJsonObject);
        {
            if (ComboMode == EComboMode::ECHAT_TTS)
            {
                ChatRequest.GenerateAttributesJson(Attributes);
                TTSRequest.GenerateAttributesJson(Attributes);
            }
            else if (ComboMode == EComboMode::ECHAT_TTS_ATL)
            {
                ChatRequest.GenerateAttributesJson(Attributes);
                TTSRequest.GenerateAttributesJson(Attributes);
                ATLRequest.GenerateAttributesJson(Attributes);
            }
            else if (ComboMode == EComboMode::ETTS_ATL)
            {
                TTSRequest.GenerateAttributesJson(Attributes);
                ATLRequest.GenerateAttributesJson(Attributes);
            }
        }
    }

    virtual FString GetRequestType() const
    {
        return TEXT("combo");
    }
    virtual FString GetAttributeName() const
    {
        return TEXT("combo");
    }

    EComboMode ComboMode;
    FTTSRequest TTSRequest;
    FATLRequest ATLRequest;
    FChatRequest ChatRequest;
};