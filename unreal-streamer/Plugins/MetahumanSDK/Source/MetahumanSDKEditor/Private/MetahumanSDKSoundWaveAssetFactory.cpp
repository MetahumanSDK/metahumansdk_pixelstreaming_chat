#include "MetahumanSDKSoundWaveAssetFactory.h"

#include "MetahumanSDKAPIInput.h"
#include "MetahumanSDKResponses.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKRequestManager.h"
#include "MetahumanSDKSettings.h"
#include "MetahumanSDKUtils.h"
#include "HttpModule.h"
#include "HttpManager.h"
#include "HttpModule.h"
#include "MetahumanSDKSoundWaveGenerateOptions.h"
#include "MetahumanSDKSoundWaveGenerateOptionsWindow.h"
#include "SoundWaveFactory.h"
#include "Interfaces/IMainFrameModule.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "MetahumanSDKSoundWaveAssetFactory"

int UMetahumanSDKSoundWaveAssetFactory::PrevSize = 0;
double UMetahumanSDKSoundWaveAssetFactory::PrevTime = 0;

DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKAssetFactory, Log, All);
#define VAL(str) #str
#define TOSTRING(str) VAL(str)

UMetahumanSDKSoundWaveAssetFactory::UMetahumanSDKSoundWaveAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Configure the class that this factory creates
	SupportedClass = USoundWave::StaticClass();

	// This factory does not manufacture new objects from scratch.
	bCreateNew = false;

	// This factory will open the editor for each new object.
	bEditAfterNew = false;

	// This factory will import objects from files.
	bEditorImport = false;

	// Factory does not import objects from text.
	bText = false;
}

UObject* UMetahumanSDKSoundWaveAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	FString Error;

	// show import options dialog
    TSharedPtr<SWindow> ParentWindow;

    if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
    {
        IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
        ParentWindow = MainFrame.GetParentWindow();
    }

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(LOCTEXT("MetahumanSDKGenerateOptions", "MetahumanSDK Generate Speech from Text"))
        .SizingRule(ESizingRule::Autosized);

	// Set source sound asset
	UMetahumanSDKSoundWaveGenerateOptions* GenerateOptions = GetMutableDefault<UMetahumanSDKSoundWaveGenerateOptions>();
    TSharedPtr<SMetahumanSDKSoundWaveGenerateOptionsWindow> OptionsWindow;
    Window->SetContent
    (
        SAssignNew(OptionsWindow, SMetahumanSDKSoundWaveGenerateOptionsWindow)
        .GenerateOptions(GenerateOptions)
        .WidgetWindow(Window)
    );

    FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

    if (!OptionsWindow->ShouldGenerate())
    {
        UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Generation aborted by user"));
        return nullptr;
    }

	//-------------------
	// Make blocking API Request
	//-------------------

	
	FMetahumanSDKTTSInput TTSInput;
	TTSInput.TTSEngine = OptionsWindow->GetEngine();
	TTSInput.VoiceID = OptionsWindow->GetVoice();
	TTSInput.Text = OptionsWindow->GetText();

	FTTSRequest TTSRequest(TTSInput);
	
	MakeTTSRequest(
		TTSRequest,
		FOnTTSRequestCompleted::CreateLambda(
			[this](TSharedPtr<FGenericResponse> Response)
			{
				LastTTSResponse = Response;
			}
		)
		);
	
	if (!LastTTSResponse.IsValid())
	{
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("Invalid response! Probably flush timeout, check your HTTP settings"), *LastTTSResponse->Error);
		return nullptr;
	}
	
	if (!LastTTSResponse->bSuccess)
	{
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("Invalid response! Error: %s"), *LastTTSResponse->Error);
		return nullptr;
	}

	FTTSResponse* TTSResponse = static_cast<FTTSResponse*>(LastTTSResponse.Get());
	check(TTSResponse != nullptr);

	//-------------------

    USoundWave* SoundWave = FSoundWaveFactory::CreateSoundWave(TTSResponse->Data, Error, InParent, InName.ToString());

	if (!IsValid(SoundWave))
	{
		Error = FString::Printf(TEXT("Can't create SoundWave from text. Error: %s"), *Error);
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("%s"), *Error);
		return nullptr;
	}

	// Broadcast notification that the new asset has been imported.
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, SoundWave);

	return SoundWave;
}

FText UMetahumanSDKSoundWaveAssetFactory::GetDisplayName() const
{
	return LOCTEXT("MetahumanSDKSoundWaveAssetFactoryDescription", "MetahumanSDK Text-to-Speech");
}

bool UMetahumanSDKSoundWaveAssetFactory::DoesSupportClass(UClass* Class)
{
	return Class == SupportedClass;
}

bool UMetahumanSDKSoundWaveAssetFactory::FactoryCanImport(const FString& Filename)
{
	return false;
}


template<typename ResponseType>
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
void ProcessResponse(const FString& RequestType, TSharedPtr<IHttpRequest> HttpRequest, FOnATLRequestCompleted CompletedDelegate)
#else 
void ProcessResponse(const FString& RequestType, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest, FOnTTSRequestCompleted CompletedDelegate)
#endif
{
    UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Request (type:'%s') started..."), *RequestType);

    const double StartTime = FPlatformTime::Seconds();
    
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [RequestType, StartTime, CompletedDelegate](FHttpRequestPtr Request, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully)
        {
            UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Request (type:'%s') finished!"), *RequestType);

            TSharedPtr<FGenericResponse> OutResponse(new ResponseType);

            const double EndTime = FPlatformTime::Seconds();
            const double RequestDuration = EndTime - StartTime;

            UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Request (type:'%s') took %f seconds to complete"), *RequestType, RequestDuration);

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
                UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Parsing response took %f seconds to complete"), ParseResponseDuration);
            }
            else
            {
                OutResponse->bSuccess = false;
                OutResponse->Error = HttpResponse->GetContentAsString();
                UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("Request error (code: %d): %s"), HttpResponse->GetResponseCode(), *OutResponse->Error);
            }

            if (CompletedDelegate.IsBound())
            {
                CompletedDelegate.Execute(OutResponse);
            }

        	UMetahumanSDKSoundWaveAssetFactory::PrevTime = RequestDuration;
			UMetahumanSDKSoundWaveAssetFactory::PrevSize = HttpResponse->GetContent().Num();
        }
    );
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

bool UMetahumanSDKSoundWaveAssetFactory::GenerateAPIToken(FString& OutAPIToken)
{
	FString _GeneratedAPIToken = "";

	TWeakObjectPtr<UMetahumanSDKSettings> MetahumanSDKSettings = GetMutableDefault<UMetahumanSDKSettings>();
	
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
        FHttpModule::Get().SetHttpTimeout(60.0);

        HttpRequest->SetURL(MetahumanSDKSettings->APIURL / TEXT("dh/api/v1/guest-token?engine=unreal"));
        HttpRequest->SetVerb(TEXT("POST"));

        UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("API token request started..."));

	    const double StartTime = FPlatformTime::Seconds();
	    
	    HttpRequest->OnProcessRequestComplete().BindLambda(
	        [&_GeneratedAPIToken, StartTime](FHttpRequestPtr Request, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully)
	        {
	            UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("API token request finished!"));

	            TSharedPtr<FGenericResponse> OutResponse(new FAPITokenResponse);

	            const double EndTime = FPlatformTime::Seconds();
	            const double RequestDuration = EndTime - StartTime;

	            UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("API token request took %f seconds to complete"), RequestDuration);

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
	                UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Parsing response took %f seconds to complete"), ParseResponseDuration);
	            }
	            else
	            {
	                OutResponse->bSuccess = false;
	                OutResponse->Error = HttpResponse->GetContentAsString();
	                UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("Request error: %s"), *OutResponse->Error);
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



void UMetahumanSDKSoundWaveAssetFactory::MakeTTSRequest(const FTTSRequest& Request, FOnTTSRequestCompleted CompletedDelegate /*= nullptr*/)
{
	if (GetMutableDefault<UMetahumanSDKSettings>() && GetMutableDefault<UMetahumanSDKSettings>()->APIToken.IsEmpty() && GenerateAPIToken(GetMutableDefault<UMetahumanSDKSettings>()->APIToken))
	{
		GetMutableDefault<UMetahumanSDKSettings>()->TryUpdateDefaultConfigFile();
	}
	
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
	{
		TWeakObjectPtr<UMetahumanSDKSettings> MetahumanSDKSettings = GetMutableDefault<UMetahumanSDKSettings>();
		
		FHttpModule::Get().SetHttpTimeout(MetahumanSDKSettings->TTSRequestTimeoutSeconds);

		const FString& ApiBaseUrl = MetahumanSDKSettings->APIURL;
		const FString& ApiToken = MetahumanSDKSettings->APIToken;
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
	FHttpModule::Get().GetHttpManager().Flush(EHttpFlushReason::Default);
}



FString UMetahumanSDKSoundWaveAssetFactory::GenerateRequestJson(const FGenericRequest& Request) const
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
			MetaJson->SetStringField(TEXT("prev_type"), TEXT("atl"));
			MetaJson->SetStringField(TEXT("prev_size"), FString::FromInt(0));
			MetaJson->SetStringField(TEXT("latency"), FString::SanitizeFloat(0.01f));
		}
		RootJson->SetObjectField(TEXT("meta"), MetaJson);
	}

	return FJsonToStringUtilities::JsonObjectToString(RootJson);
}