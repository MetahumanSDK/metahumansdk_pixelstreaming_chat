#include "MetahumanSDKAnimSequenceAssetFactory.h"
#include "AssetTypeCategories.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Editor/EditorEngine.h"
#include "Curves/RealCurve.h"
#include "Interfaces/IMainFrameModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetFactories/AnimSequenceFactory.h"
#include "MetahumanSDKAPIInput.h"
#include "MetahumanSDKResponses.h"
#include "MetahumanSDKAnimSequenceImportOptions.h"
#include "MetahumanSDKAnimSequenceImportOptionsWindow.h"
#include "MetahumanSDKAnimSequenceGenerateOptions.h"
#include "MetahumanSDKAnimSequenceGenerateOptionsWindow.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKRequestManager.h"
#include "MetahumanSDKSettings.h"
#include "MetahumanSDKUtils.h"
#include "HttpManager.h"
#include "HttpModule.h"
#include "Async/Async.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Misc/ScopedSlowTask.h"
#endif

#define LOCTEXT_NAMESPACE "MetahumanSDKAnimSequenceAssetFactory"

DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKAssetFactory, Log, All);

#define VAL(str) #str
#define TOSTRING(str) VAL(str)


int UMetahumanSDKAnimSequenceAssetFactory::PrevSize = 0;
double UMetahumanSDKAnimSequenceAssetFactory::PrevTime = 0;
double UMetahumanSDKAnimSequenceAssetFactory::PrevAudioLength = 0;

UMetahumanSDKAnimSequenceAssetFactory::UMetahumanSDKAnimSequenceAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Configure the class that this factory creates
	SupportedClass = UAnimSequence::StaticClass();

	// This factory does not manufacture new objects from scratch.
	bCreateNew = false;

	// This factory will open the editor for each new object.
	bEditAfterNew = false;

	// This factory will import objects from files.
	bEditorImport = true;

	// Factory does not import objects from text.
	bText = true;

	// Add supported formats.
	Formats.Add(FString(TEXT("json;")) + NSLOCTEXT("MetahumanSDKJSON", "FormatJSON", "JSON File").ToString());
}

UObject* UMetahumanSDKAnimSequenceAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	bOutOperationCanceled = false;

	FString Error;

	FString DataString;
	if (!FFileHelper::LoadFileToString(DataString, *Filename))
	{
		Error = FString::Printf(TEXT("Filename %s does not exist: %s"), *Filename);
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("%s"), *Error);
		bOutOperationCanceled = true;
		return nullptr;
	}

	FATLResponse ATLResponse;
	ATLResponse.ParseResponse(MoveTemp(DataString));
	if (!ATLResponse.bSuccess)
	{
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("Invalid json file!"));
		bOutOperationCanceled = true;
		return nullptr;
	}

	// show import options dialog
    TSharedPtr<SWindow> ParentWindow;

    if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
    {
        IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
        ParentWindow = MainFrame.GetParentWindow();
    }

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(LOCTEXT("MetahumanSDKJSONImportOptions", "MetahumanSDK JSON Import Options"))
        .SizingRule(ESizingRule::Autosized);


    TSharedPtr<SMetahumanSDKAnimSequenceImportOptionsWindow> OptionsWindow;
    Window->SetContent
    (
        SAssignNew(OptionsWindow, SMetahumanSDKAnimSequenceImportOptionsWindow)
        .ImportOptions(GetMutableDefault<UMetahumanSDKAnimSequenceImportOptions>())
        .WidgetWindow(Window)
    );

    FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

    if (!OptionsWindow->ShouldImport())
    {
        UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Import aborted by user"));
		bOutOperationCanceled = true;
        return nullptr;
    }

	const FString& AnimationAssetName = FPaths::GetBaseFilename(Filename);

    UAnimSequence* AnimSequence = FAnimSequenceFactory::CreateAnimSequence(
        OptionsWindow->GetSkeleton(),
        OptionsWindow->GetBlendShapeCurveInterpolationMode(),
        FATLMappingsInfo(OptionsWindow->GetMappingsInfo()),
        &ATLResponse, Error, InParent, AnimationAssetName
    );

	if (!IsValid(AnimSequence))
	{
		Error = FString::Printf(TEXT("Can't create AnimSequence from file: %s. Error: %s"), *Filename, *Error);
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("%s"), *Error);
		bOutOperationCanceled = true;
		return nullptr;
	}

	// Broadcast notification that the new asset has been imported.
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, AnimSequence);

	return AnimSequence;
}

UObject* UMetahumanSDKAnimSequenceAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
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
        .Title(LOCTEXT("MetahumanSDKGenerateOptions", "MetahumanSDK Generate Lipsync Animation"))
        .SizingRule(ESizingRule::Autosized);

	// Set source sound asset
	UMetahumanSDKAnimSequenceGenerateOptions* GenerateOptions = GetMutableDefault<UMetahumanSDKAnimSequenceGenerateOptions>();
	if (SourceSoundWave.IsValid())
	{
		GenerateOptions->Sound = SourceSoundWave.Get();
	}
	
    TSharedPtr<SMetahumanSDKAnimSequenceGenerateOptionsWindow> OptionsWindow;
    Window->SetContent
    (
        SAssignNew(OptionsWindow, SMetahumanSDKAnimSequenceGenerateOptionsWindow)
        .GenerateOptions(GenerateOptions)
        .WidgetWindow(Window)
    );

    FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

    if (!OptionsWindow->ShouldGenerate())
    {
        UE_LOG(LogMetahumanSDKAssetFactory, Log, TEXT("Import aborted by user"));
        return nullptr;
    }

	//-------------------
	// Make blocking API Request
	//-------------------

	
	FMetahumanSDKATLInput ATLInput;
	ATLInput.Sound = OptionsWindow->GetSound();
	ATLInput.ExplicitEmotion = OptionsWindow->GetExplicitEmotion();
	ATLInput.EyeMovementModeMocap = OptionsWindow->GetGenerateEyeMovement();
	ATLInput.NeckMovementModeMocap = OptionsWindow->GetGenerateNeckMovement();
	ATLInput.MappingsInfo = OptionsWindow->GetMappingsInfo();
	
	FATLRequest ATLRequest(ATLInput, true);
	if(!ATLRequest.bSuccessful)
	{
		Error = FString::Printf(TEXT("Can't prepare ATL editor request with provided sound wave!"));
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("%s -- %s"), *FString(__FUNCTION__), *Error);
		return nullptr;
	}
	
	MakeATLRequest(
		ATLRequest,
		FOnATLRequestCompleted::CreateLambda(
			[this](TSharedPtr<FGenericResponse> Response)
			{
				LastATLResponse = Response;
			}
		)
	);
	
	if (!LastATLResponse.IsValid() || !LastATLResponse->bSuccess)
	{
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("Invalid response! Error: %s"), *LastATLResponse->Error);
		return nullptr;
	}

	FATLResponse* ATLResponse = static_cast<FATLResponse*>(LastATLResponse.Get());
	check(ATLResponse != nullptr);

	//-------------------

    UAnimSequence* AnimSequence = FAnimSequenceFactory::CreateAnimSequence(
        OptionsWindow->GetSkeleton(),
        OptionsWindow->GetBlendShapeCurveInterpolationMode(),
        FATLMappingsInfo(OptionsWindow->GetMappingsInfo()),
        ATLResponse, Error, InParent, InName.ToString()
    );

	if (!IsValid(AnimSequence))
	{
		Error = FString::Printf(TEXT("Can't create AnimSequence from SoundWave. Error: %s"), *Error);
		UE_LOG(LogMetahumanSDKAssetFactory, Error, TEXT("%s"), *Error);
		return nullptr;
	}

	// Broadcast notification that the new asset has been imported.
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, AnimSequence);

	return AnimSequence;
}

FText UMetahumanSDKAnimSequenceAssetFactory::GetDisplayName() const
{
	return LOCTEXT("MetahumanSDKAnimSequenceAssetFactoryDescription", "MetahumanSDK Animation JSON File");
}

bool UMetahumanSDKAnimSequenceAssetFactory::DoesSupportClass(UClass* Class)
{
	return Class == SupportedClass;
}

bool UMetahumanSDKAnimSequenceAssetFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

    FString DataString;
	if (!FFileHelper::LoadFileToString(DataString, *Filename))
	{
		return false;
	}

	if (Extension == TEXT("json"))
	{
        FATLResponse ATLResponse;
        return ATLResponse.IsValidJsonString(DataString);
	}

	return false;
}


template<typename ResponseType>
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
void ProcessResponse(const FString& RequestType, TSharedPtr<IHttpRequest> HttpRequest, FOnATLRequestCompleted CompletedDelegate)
#else 
void ProcessResponse(const FString& RequestType, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest, FOnATLRequestCompleted CompletedDelegate)
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

        	UMetahumanSDKAnimSequenceAssetFactory::PrevTime = RequestDuration;
			UMetahumanSDKAnimSequenceAssetFactory::PrevSize = HttpResponse->GetContent().Num();
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

bool UMetahumanSDKAnimSequenceAssetFactory::GenerateAPIToken(FString& OutAPIToken)
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

void UMetahumanSDKAnimSequenceAssetFactory::MakeATLRequest(const FATLRequest& Request, FOnATLRequestCompleted CompletedDelegate /*= nullptr*/)
{
#if WITH_EDITOR
	FScopedSlowTask SlowTask(1, INVTEXT("Generating lipsync..."));
	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame(1);
#endif
	
	TWeakObjectPtr<UMetahumanSDKSettings> MetahumanSDKSettings = GetMutableDefault<UMetahumanSDKSettings>();
	
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
    TSharedPtr<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#else
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
    {
		if (MetahumanSDKSettings.IsValid() && MetahumanSDKSettings->APIToken.IsEmpty() && GenerateAPIToken(MetahumanSDKSettings->APIToken))
		{
			MetahumanSDKSettings->TryUpdateDefaultConfigFile();
		}
		
        FHttpModule::Get().SetHttpTimeout(MetahumanSDKSettings->ATLRequestTimeoutSeconds);
        
        const FString& ApiBaseUrl = MetahumanSDKSettings->APIURL;
        const FString& ApiToken = MetahumanSDKSettings->APIToken;
        const FString& FullURL = ApiBaseUrl / TEXT("dh/api/v2/atl");
        
        const static FString FORM_BOUNDARY("ATLBoundary");

        HttpRequest->SetURL(FullURL);
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetHeader(TEXT("Authorization"), ApiToken);
        HttpRequest->SetHeader(TEXT("Accept-Encoding"), TEXT("utf-8"));
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

        ProcessResponse<FATLResponse>(Request.GetRequestType(), HttpRequest, CompletedDelegate);
    }

    HttpRequest->ProcessRequest();
	FHttpModule::Get().GetHttpManager().Flush(EHttpFlushReason::Default);

	PrevAudioLength = Request.ATLInput.Sound->GetDuration();
}

FString UMetahumanSDKAnimSequenceAssetFactory::GenerateRequestJson(const FGenericRequest& Request) const
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
			MetaJson->SetStringField(TEXT("prev_size"), FString::FromInt(UMetahumanSDKAnimSequenceAssetFactory::PrevSize));
			MetaJson->SetStringField(TEXT("latency"), FString::SanitizeFloat(UMetahumanSDKAnimSequenceAssetFactory::PrevTime));
            MetaJson->SetNumberField(TEXT("prev_audio_length"), UMetahumanSDKAnimSequenceAssetFactory::PrevAudioLength);
		}
		RootJson->SetObjectField(TEXT("meta"), MetaJson);
	}

	return FJsonToStringUtilities::JsonObjectToString(RootJson);
}

#undef LOCTEXT_NAMESPACE