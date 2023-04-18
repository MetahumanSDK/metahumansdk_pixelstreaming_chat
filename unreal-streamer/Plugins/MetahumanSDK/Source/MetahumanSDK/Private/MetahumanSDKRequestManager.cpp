#include "MetahumanSDKRequestManager.h"

#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "HAL/Event.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "PlatformHttp.h"
#include "HttpModule.h"
#include "HttpManager.h"

#include "MetahumanSDKSettings.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKUtils.h"
#include "Internationalization/Regex.h"
#include "Internationalization/Culture.h"
#include "Kismet/GameplayStatics.h"

#define VAL(str) #str
#define TOSTRING(str) VAL(str)

DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKRequestsManager, Log, All);


FString FMetahumanSDKRequestManager::PrevType = TEXT("");
bool FMetahumanSDKRequestManager::PrevComboAtl = false;
bool FMetahumanSDKRequestManager::PrevComboChat = false;
bool FMetahumanSDKRequestManager::PrevComboTts = false;
int FMetahumanSDKRequestManager::PrevSize = 0;
double FMetahumanSDKRequestManager::PrevTime = 0;
double FMetahumanSDKRequestManager::PrevAudioLength = 0;

FMetahumanSDKRequestManager::FMetahumanSDKRequestManager(UMetahumanSDKAPIManager* InAPIManager)
    : APIManager(InAPIManager)
{
//    Buffer = NewObject<UATLStreamBuffer>();
}

FMetahumanSDKRequestManager::~FMetahumanSDKRequestManager()
{
    Cleanup();
}

void FMetahumanSDKRequestManager::Tick(float DeltaTime)
{
    // 
}

TStatId FMetahumanSDKRequestManager::GetStatId() const
{
    return TStatId();
}

void FMetahumanSDKRequestManager::Initialize()
{
    WorkerThread = FRunnableThread::Create(this, TEXT("MetahumanSDKRequestsManager"));

    MetahumanSDKSettings = GetMutableDefault<UMetahumanSDKSettings>();
}

void FMetahumanSDKRequestManager::Cleanup()
{
    bStopThread = true;
    Trigger();
    WorkerThread->WaitForCompletion();
    //bCompletedWork.AtomicSet(false);
}

bool FMetahumanSDKRequestManager::Init()
{
    WorkerThreadSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);

    return true;
}

uint32 FMetahumanSDKRequestManager::Run()
{
    while (!bStopThread)
    {
        WorkerThreadSemaphore->Wait();
        
        BlockTillAllRequestsFinished();
    }

    return 0;
}

void FMetahumanSDKRequestManager::Exit()
{
    bStopThread = true;
    bCompletedWork.AtomicSet(true);
}

void FMetahumanSDKRequestManager::Trigger()
{
    WorkerThreadSemaphore->Trigger();
}

void FMetahumanSDKRequestManager::BlockTillAllRequestsFinished()
{
    while (!bCompletedWork && !bStopThread)
    {
        bCompletedWork.AtomicSet(true);
    }
}

template<typename ResponseType>
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
void ProcessResponse(const FString& RequestType, TSharedPtr<IHttpRequest> HttpRequest, FOnDHRequestCompleted CompletedDelegate)
#else 
void ProcessResponse(const FString& RequestType, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest, FOnDHRequestCompleted CompletedDelegate)
#endif
{
    UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("Request (type:'%s') started..."), *RequestType);

    const double StartTime = FPlatformTime::Seconds();
    
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [RequestType, StartTime, CompletedDelegate](FHttpRequestPtr Request, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully)
        {
            UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("Request (type:'%s') finished!"), *RequestType);

            TSharedPtr<FGenericResponse> OutResponse(new ResponseType);

            const double EndTime = FPlatformTime::Seconds();
            const double RequestDuration = EndTime - StartTime;

            UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("Request (type:'%s') took %f seconds to complete"), *RequestType, RequestDuration);

            // Handle critical error peacefully to avoid crash
            if (HttpResponse != nullptr && HttpResponse->GetResponseCode() == EHttpResponseCodes::Ok)
            {
                const double ParseResponseStartTime = FPlatformTime::Seconds();
                if (OutResponse->PassContentAsBytes())
                {
                    OutResponse->ParseResponse(HttpResponse->GetContent());                   
                }
                else
                {
                    OutResponse->ParseResponse(HttpResponse->GetContentAsString());
                }

                const double ParseResponseDuration = FPlatformTime::Seconds() - ParseResponseStartTime;
                UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("Parsing response took %f seconds to complete"), ParseResponseDuration);
            }
            else
            {
                OutResponse->bSuccess = false;
                OutResponse->Error = HttpResponse->GetContentAsString();
                UE_LOG(LogMetahumanSDKRequestsManager, Error, TEXT("Request response code: %d"), HttpResponse->GetResponseCode());
                UE_LOG(LogMetahumanSDKRequestsManager, Error, TEXT("Request error: %s"), *OutResponse->Error);
            }

            if (CompletedDelegate.IsBound())
            {
                CompletedDelegate.Execute(OutResponse, bConnectedSuccessfully);
            }
            
            FMetahumanSDKRequestManager::PrevTime = RequestDuration;
            FMetahumanSDKRequestManager::PrevSize = HttpResponse->GetContent().Num();
        }
    );
}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
TSharedPtr<IHttpRequest> FMetahumanSDKRequestManager::MakeATLRequest(const FATLRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FMetahumanSDKRequestManager::MakeATLRequest(const FATLRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
        if (GetMutableDefault<UMetahumanSDKSettings>() && GetMutableDefault<UMetahumanSDKSettings>()->APIToken.IsEmpty() && GenerateAPIToken(GetMutableDefault<UMetahumanSDKSettings>()->APIToken))
        {
            GetMutableDefault<UMetahumanSDKSettings>()->TryUpdateDefaultConfigFile();
        }
        
        FHttpModule::Get().SetHttpTimeout(GetMutableDefault<UMetahumanSDKSettings>()->ATLRequestTimeoutSeconds);
        
        const FString& ApiBaseUrl = GetMutableDefault<UMetahumanSDKSettings>()->APIURL;
        const FString& ApiToken = GetMutableDefault<UMetahumanSDKSettings>()->APIToken;
        const FString& FullURL = ApiBaseUrl / TEXT("dh/api/v2/atl");
        
        const static FString FORM_BOUNDARY("ATLBoundary");

        HttpRequest->SetURL(FullURL);
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetHeader(TEXT("Authorization"), ApiToken);
        HttpRequest->SetHeader("Content-Type", FString::Printf(TEXT("multipart/form-data; boundary=%s"), *FORM_BOUNDARY));
        HttpRequest->SetHeader(TEXT("Accept-Encoding"), TEXT("utf-8"));        

        const FString Line0 = FString::Printf(TEXT("\r\n--%s"), *FORM_BOUNDARY);
        const FString Line1 = TEXT("\r\nContent-Disposition: form-data; name=\"json\"\r\n\r\n");
        const FString Line2 = GenerateRequestJson(Request);
        const FString Line3 = FString::Printf(TEXT("\r\n--%s"), *FORM_BOUNDARY);
        const FString Line4 = TEXT("\r\nContent-Type: application/octet-stream");
        const FString Line5 = TEXT("\r\nContent-Disposition: form-data; name=\"audio\"; filename=\"audio.wav\"\r\n\r\n");
        const FString Line6 = FString::Printf(TEXT("\r\n--%s--\r\n"), *FORM_BOUNDARY);

        TArray<uint8> ContentData;
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line0), Line0.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line1), Line1.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line2), Line2.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line3), Line3.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line4), Line4.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line5), Line5.Len());
        ContentData.Append(Request.AudioData);
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line6), Line6.Len());

        HttpRequest->SetContent(ContentData);

        ProcessResponse<FATLResponse>(Request.GetRequestType(), HttpRequest, CompletedDelegate);
    }

    HttpRequest->ProcessRequest();
    PrevType = TEXT("atl");
    PrevComboAtl = false;
    PrevComboChat = false;
    PrevComboTts = false;
    PrevAudioLength = Request.ATLInput.Sound->GetDuration();

    return HttpRequest;
}

struct FAPITokenResponse : public FGenericResponse
{
    virtual void ParseResponse(FString&& Content) override
    {
        TSharedRef<TJsonReader<>> Reader = FJsonStringReader::Create(MoveTemp(Content));

        TSharedPtr<FJsonObject> RootJson;
        if (FJsonSerializer::Deserialize(Reader, RootJson))
        {
            RootJson->TryGetStringField(TEXT("token"), Token);
        }

        if (!Validate())
        {
            bSuccess = false;
            Error = FString::Printf(TEXT("Invalid API token response json: %s"), *Content);
        }
    }

    virtual bool PassContentAsBytes() const override
    {
        return false;
    }

    bool Validate() const
    {
        return Token.Len() > 0;
    }

    FString Token = "";
};

bool FMetahumanSDKRequestManager::GenerateAPIToken(FString& OutAPIToken)
{
	FString _GeneratedAPIToken = "";
	
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
        FHttpModule::Get().SetHttpTimeout(60.0);

        HttpRequest->SetURL(GetMutableDefault<UMetahumanSDKSettings>()->APIURL / TEXT("dh/api/v1/guest-token?engine=unreal"));
        HttpRequest->SetVerb(TEXT("POST"));

        UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("API token request started..."));

	    const double StartTime = FPlatformTime::Seconds();
	    
	    HttpRequest->OnProcessRequestComplete().BindLambda(
	        [&_GeneratedAPIToken, StartTime](FHttpRequestPtr Request, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully)
	        {
	            UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("API token request finished!"));

	            TSharedPtr<FGenericResponse> OutResponse(new FAPITokenResponse);

	            const double EndTime = FPlatformTime::Seconds();
	            const double RequestDuration = EndTime - StartTime;

	            UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("API token request took %f seconds to complete"), RequestDuration);
	            UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("DHS HTTP BODY: %s"), *HttpResponse->GetContentAsString());
	            // Handle critical error peacefully to avoid crash
	            if (HttpResponse != nullptr && HttpResponse->GetResponseCode() == EHttpResponseCodes::Ok)
	            {
	                const double ParseResponseStartTime = FPlatformTime::Seconds();
	                if (OutResponse->PassContentAsBytes())
	                {
	                    OutResponse->ParseResponse(HttpResponse->GetContent());                   
	                }
	                else
	                {
	                    OutResponse->ParseResponse(HttpResponse->GetContentAsString());
	                }

	                const double ParseResponseDuration = FPlatformTime::Seconds() - ParseResponseStartTime;
	                UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("Parsing response took %f seconds to complete"), ParseResponseDuration);
	            }
	            else
	            {
	                OutResponse->bSuccess = false;
	                OutResponse->Error = HttpResponse->GetContentAsString();
	                UE_LOG(LogMetahumanSDKRequestsManager, Error, TEXT("Request error: %s"), *OutResponse->Error);
	            }

	        	FAPITokenResponse* APITokenResponse = static_cast<FAPITokenResponse*>(OutResponse.Get());
				check(APITokenResponse != nullptr);
	        	_GeneratedAPIToken = APITokenResponse->Token;
	        }
	    );
    }
	HttpRequest->ProcessRequest();
	FHttpModule::Get().GetHttpManager().Flush(EHttpFlushReason::Default);

	OutAPIToken = _GeneratedAPIToken;

	return true;
}


#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
TSharedPtr<IHttpRequest> FMetahumanSDKRequestManager::MakeTTSRequest(const FTTSRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FMetahumanSDKRequestManager::MakeTTSRequest(const FTTSRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
        FHttpModule::Get().SetHttpTimeout(GetMutableDefault<UMetahumanSDKSettings>()->TTSRequestTimeoutSeconds);

        const FString& ApiBaseUrl = GetMutableDefault<UMetahumanSDKSettings>()->APIURL;
        const FString& ApiToken = GetMutableDefault<UMetahumanSDKSettings>()->APIToken;
        const FString& FullURL = ApiBaseUrl / TEXT("dh/api/v1/tts");

        HttpRequest->SetURL(FullURL);
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetHeader(TEXT("Authorization"), ApiToken);
        HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
        HttpRequest->SetHeader(TEXT("Accept-Encoding"), TEXT("utf-8"));

        HttpRequest->SetContentAsString(GenerateRequestJson(Request));

        ProcessResponse<FTTSResponse>(Request.GetRequestType(), HttpRequest, CompletedDelegate);
    }

    HttpRequest->ProcessRequest();
    PrevType = TEXT("tts");
    PrevComboAtl = false;
    PrevComboChat = false;
    PrevComboTts = false;
    
    return HttpRequest;
}


#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
TSharedPtr<IHttpRequest> FMetahumanSDKRequestManager::MakeChatRequest(const FChatRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FMetahumanSDKRequestManager::MakeChatRequest(const FChatRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
        FHttpModule::Get().SetHttpTimeout(GetMutableDefault<UMetahumanSDKSettings>()->ChatRequestTimeoutSeconds);
        
        const FString& ApiBaseUrl = GetMutableDefault<UMetahumanSDKSettings>()->APIURL;
        const FString& ApiToken = GetMutableDefault<UMetahumanSDKSettings>()->APIToken;
        const FString& FullURL = ApiBaseUrl / TEXT("dh/api/v1/chat");

        const FString& FORM_BOUNDARY("ATLBoundary");

        HttpRequest->SetURL(FullURL);
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetHeader(TEXT("Authorization"), ApiToken);
        HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
        HttpRequest->SetHeader(TEXT("Accept-Encoding"), TEXT("utf-8"));

        HttpRequest->SetContentAsString(GenerateRequestJson(Request));

        ProcessResponse<FChatResponse>(Request.GetRequestType(), HttpRequest, CompletedDelegate);
    }

    HttpRequest->ProcessRequest();
    PrevType = TEXT("chat");
    PrevComboAtl = false;
    PrevComboChat = false;
    PrevComboTts = false;
    
    return HttpRequest;
}


#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
TSharedPtr<IHttpRequest> FMetahumanSDKRequestManager::MakeComboRequest(const FComboRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FMetahumanSDKRequestManager::MakeComboRequest(const FComboRequest& Request, FOnDHRequestCompleted CompletedDelegate /*= nullptr*/)
{
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
        FHttpModule::Get().SetHttpTimeout(GetMutableDefault<UMetahumanSDKSettings>()->ComboRequestTimeoutSeconds);

        const FString& ApiBaseUrl = GetMutableDefault<UMetahumanSDKSettings>()->APIURL;
        const FString& ApiToken = GetMutableDefault<UMetahumanSDKSettings>()->APIToken;
        const FString& FullURL = ApiBaseUrl / TEXT("dh/api/v1/combo");
        UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("Request %s"), *ApiToken);
        HttpRequest->SetURL(FullURL);
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetHeader(TEXT("Authorization"), ApiToken);
        HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
        HttpRequest->SetHeader(TEXT("Accept-Encoding"), TEXT("utf-8"));

        HttpRequest->SetContentAsString(GenerateRequestJson(Request));

        ProcessResponse<FComboResponse>(Request.GetRequestType(), HttpRequest, CompletedDelegate);
    }

    HttpRequest->ProcessRequest();
    PrevType = TEXT("combo");
    PrevComboAtl = Request.ComboMode == EComboMode::ECHAT_TTS_ATL || Request.ComboMode == EComboMode::ETTS_ATL;
    PrevComboChat = Request.ComboMode == EComboMode::ECHAT_TTS_ATL || Request.ComboMode == EComboMode::ECHAT_TTS;
    PrevComboTts = Request.ComboMode == EComboMode::ECHAT_TTS_ATL || Request.ComboMode == EComboMode::ETTS_ATL || Request.ComboMode == EComboMode::ETTS_ATL;
    
    return HttpRequest;
}

FString FMetahumanSDKRequestManager::GenerateRequestJson(const FGenericRequest& Request) const
{
    TSharedPtr<FJsonObject> RootJson = MakeShareable<FJsonObject>(new FJsonObject);
    {
        TSharedPtr<FJsonObject> DataJson = MakeShareable<FJsonObject>(new FJsonObject);
        {
            DataJson->SetStringField(TEXT("id"), FGuid::NewGuid().ToString());
            DataJson->SetStringField(TEXT("type"), Request.GetRequestType());

            TSharedPtr<FJsonObject> AttributesJson = MakeShareable<FJsonObject>(new FJsonObject);
            {
                TMap<FString, TSharedPtr<FJsonObject>> Attributes;
                Request.GenerateAttributesJson(Attributes);
                ensure (Attributes.Num() > 0);

                for (const TTuple<FString, TSharedPtr<FJsonObject>>& Attribute : Attributes)
                {
                    AttributesJson->SetObjectField(Attribute.Key, Attribute.Value);
                }
            }
            DataJson->SetObjectField(TEXT("attributes"), AttributesJson);
        }
        RootJson->SetObjectField(TEXT("data"), DataJson);

        TSharedPtr<FJsonObject> MetaJson = MakeShareable<FJsonObject>(new FJsonObject);
        {
            MetaJson->SetStringField(TEXT("platform"), TEXT("unreal"));
            MetaJson->SetStringField(TEXT("platform_version"), TOSTRING(ENGINE_MAJOR_VERSION) "." TOSTRING(ENGINE_MINOR_VERSION));
            MetaJson->SetStringField(TEXT("plugin_version"), GetMutableDefault<UMetahumanSDKSettings>()->GetPluginVersion());
            MetaJson->SetStringField(TEXT("OS"), UGameplayStatics::GetPlatformName());
            MetaJson->SetStringField(TEXT("local"), FInternationalization::Get().GetDefaultLanguage()->GetName());
            MetaJson->SetStringField(TEXT("prev_type"), PrevType);
            MetaJson->SetStringField(TEXT("prev_size"), FString::FromInt(FMetahumanSDKRequestManager::PrevSize));
            MetaJson->SetStringField(TEXT("latency"), FString::SanitizeFloat(FMetahumanSDKRequestManager::PrevTime));
            MetaJson->SetBoolField(TEXT("prev_combo_atl"), PrevComboAtl);
            MetaJson->SetBoolField(TEXT("prev_combo_chat"), PrevComboChat);
            MetaJson->SetBoolField(TEXT("prev_combo_tts"), PrevComboTts);
            MetaJson->SetNumberField(TEXT("prev_audio_length"), PrevAudioLength);
        }
        RootJson->SetObjectField(TEXT("meta"), MetaJson);
    }

    return FJsonToStringUtilities::JsonObjectToString(RootJson);
}

#if ( ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION < 26 )
TSharedPtr<IHttpRequest> FMetahumanSDKRequestManager::MakeATLStreamRequest(const FATLRequest& Request, FOnDHRequestCompleted ChunkRecvDelegate, FOnDHRequestCompleted CompletedDelegate)
{
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FMetahumanSDKRequestManager::MakeATLStreamRequest(const FATLRequest& Request, FOnDHRequestCompleted ChunkRecvDelegate, FOnDHRequestCompleted CompletedDelegate)
{
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
        FHttpModule::Get().SetHttpTimeout(GetMutableDefault<UMetahumanSDKSettings>()->ATLRequestTimeoutSeconds);
        
        const FString& ApiBaseUrl = GetMutableDefault<UMetahumanSDKSettings>()->APIURL;
        const FString& ApiToken = GetMutableDefault<UMetahumanSDKSettings>()->APIToken;
        const FString& FullURL = ApiBaseUrl / TEXT("dh/api/v1/atl_stream");
        
        const static FString FORM_BOUNDARY("ATLBoundary");

        HttpRequest->SetURL(FullURL);
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetHeader(TEXT("Authorization"), ApiToken);
        HttpRequest->SetHeader("Content-Type", FString::Printf(TEXT("multipart/form-data; boundary=%s"), *FORM_BOUNDARY));

        const FString Line0 = FString::Printf(TEXT("\r\n--%s"), *FORM_BOUNDARY);
        const FString Line1 = TEXT("\r\nContent-Disposition: form-data; name=\"json\"\r\n\r\n");
        const FString Line2 = GenerateRequestJson(Request);
        const FString Line3 = FString::Printf(TEXT("\r\n--%s"), *FORM_BOUNDARY);
        const FString Line4 = TEXT("\r\nContent-Type: application/octet-stream");
        const FString Line5 = TEXT("\r\nContent-Disposition: form-data; name=\"audio\"; filename=\"audio.wav\"\r\n\r\n");
        const FString Line6 = FString::Printf(TEXT("\r\n--%s--\r\n"), *FORM_BOUNDARY);

        TArray<uint8> ContentData;
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line0), Line0.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line1), Line1.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line2), Line2.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line3), Line3.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line4), Line4.Len());
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line5), Line5.Len());
        ContentData.Append(Request.AudioData);
        ContentData.Append((uint8*)TCHAR_TO_UTF8(*Line6), Line6.Len());

        HttpRequest->SetContent(ContentData);

        // ProcessStreamResponse<FATLResponse>(Request.GetRequestType(), HttpRequest, ChunkRecvDelegate, CompletedDelegate);

        const double StartTime = FPlatformTime::Seconds();
        UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream Request started, t = %f seconds"), StartTime);

       
        ChunkNum = 0;
        LastProgressUpdateBytes = 0;
        HttpRequest->OnRequestProgress().BindLambda(
            [this, ChunkRecvDelegate, StartTime](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
            {
                const double EndTime = FPlatformTime::Seconds();
                UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream OnRequestProgress [BytesSent:%d | BytesReceived:%d], t = %f seconds"), BytesSent, BytesReceived, EndTime - StartTime);
                
                if (BytesReceived == 0)
                {
                    return;
                }

                if (Request->GetResponse())
                {
                    FString Content = Request->GetResponse()->GetContentAsString();

                    // UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream chunk!!! ContentLen: %d"), Content.Len());
                    // UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream chunk!!! Content: %s"), *Content);
                    // UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream chunk!!! LastProgressUpdateBytes: %d"), LastProgressUpdateBytes);
                    // UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream chunk!!! ChunkLen: %d"), Content.Len() - LastProgressUpdateBytes);
                    // UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream chunk!!! Chunk: %s"), *Content.Right(Content.Len() - LastProgressUpdateBytes));

                    bool HasSomethingToParse = true;
                    while (HasSomethingToParse)
                    {
                        HasSomethingToParse = false;
                        
                        FString ContentBuffer = Content.Right(Content.Len() - LastProgressUpdateBytes);
                    
                        // BUG: It looks like UE regexp does not support recursion...
                        //const FRegexPattern jsonPattern(TEXT("\\{(?:[^{}]|(?R))*\\}"));
                        // ... so i'll just copy regex into itself a few times
                        const FRegexPattern jsonPattern(TEXT("\\{(?:[^{}]|(\\{(?:[^{}]|(\\{(?:[^{}]|(\\{(?:[^{}])*\\}))*\\}))*\\}))*\\}"));
                        FRegexMatcher jsonPatternMatcher(jsonPattern, ContentBuffer);

                        if (jsonPatternMatcher.FindNext()) 
                        {
                            if (jsonPatternMatcher.GetCaptureGroupBeginning(0) == 0)
                            {
                                FString Chunk = jsonPatternMatcher.GetCaptureGroup(0);

                                UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream chunk [len: %d, cursor pos: %d], t = %f seconds"), Chunk.Len(), LastProgressUpdateBytes, EndTime - StartTime);

                                FString TmpFilename = FPaths::ProjectSavedDir() / TEXT("MetahumanSDK") / TEXT("Temp");
                                TmpFilename += TEXT("/");
                                TmpFilename += FString::Printf(TEXT("atl_stream_%d.json"), ChunkNum);
                                FFileHelper::SaveStringToFile(Chunk, *TmpFilename);

                                if (ChunkRecvDelegate.IsBound())
                                {
                                    TSharedPtr<FGenericResponse> OutResponse(new FATLResponse);
                                    OutResponse->ParseResponse(*Chunk);
                                    ChunkRecvDelegate.Execute(OutResponse, true);
                                }

                                ChunkNum++;
                                LastProgressUpdateBytes += Chunk.Len();

                                HasSomethingToParse = true;
                            }
                        }
                    }
                }
            }
        );
        HttpRequest->OnProcessRequestComplete().BindLambda(
            [this, StartTime, CompletedDelegate](FHttpRequestPtr Request, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully)
            {
                UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream Request finished!"));

                TSharedPtr<FGenericResponse> OutResponse(new FATLResponse);

                const double EndTime = FPlatformTime::Seconds();
                const double RequestDuration = EndTime - StartTime;

                UE_LOG(LogMetahumanSDKRequestsManager, Log, TEXT("ATL stream Request took %f seconds to complete"), RequestDuration);

                // Handle critical error peacefully to avoid crash
                if ( HttpResponse != nullptr && HttpResponse->GetResponseCode() == EHttpResponseCodes::Ok )
                {                   
                    OutResponse->bSuccess = true;
                    OutResponse->Error = TEXT("");
                }
                else
                {
                    OutResponse->bSuccess = false;                    
                    OutResponse->Error = HttpResponse->GetResponseCode() == EHttpResponseCodes::Unknown ? TEXT("Request Aborted") : HttpResponse->GetContentAsString();                    

                    UE_LOG(LogMetahumanSDKRequestsManager, Error, TEXT("ATL stream Request error: %s"), *OutResponse->Error);
                }

                if (CompletedDelegate.IsBound())
                {
                    CompletedDelegate.Execute(OutResponse, bConnectedSuccessfully);
                }

                FMetahumanSDKRequestManager::PrevTime = RequestDuration;
                FMetahumanSDKRequestManager::PrevSize = HttpResponse->GetContent().Num();
            }
        );
    }

    HttpRequest->ProcessRequest();
    PrevType = TEXT("atl_stream");
    PrevComboAtl = false;
    PrevComboChat = false;
    PrevComboTts = false;
    PrevAudioLength = Request.ATLInput.Sound->GetDuration();
    
    return HttpRequest;
}