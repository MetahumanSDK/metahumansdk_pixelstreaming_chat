#include "ComboAction.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Animation/AnimSequence.h"
#include "Sound/SoundWave.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKRequestManager.h"
#include "MetahumanSDKResponses.h"
#include "MetahumanSDKUtils.h"
#include "SoundWaveFactory.h"
#include "AnimSequenceFactory.h"


DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKComboAction, Log, All);


FComboAction::FComboAction(const FMetahumanSDKComboInput& InComboInput)
: ComboInput(InComboInput), bRequestSent(false), bRequestCompleted(false)
{
    Initialize();
}

FComboAction::~FComboAction()
{

}

void FComboAction::Initialize()
{
    bRequestSent = false;
    bRequestCompleted = false;
    HttpRequest = nullptr;
}

void FComboAction::Execute()
{
    check(GetRequestsManager().IsValid());

    if (!bRequestSent)
    {
        const double StartTime = FPlatformTime::Seconds();

        FChatRequest ChatRequest(ComboInput.ChatInput);
        {
            APIManager->CacheChatMessage(ComboInput.ChatInput.Text);
            ChatRequest.ChatID = APIManager->GetChatID();
            ChatRequest.ChatHistory = APIManager->GetChatHistory();
        }
        FTTSRequest TTSRequest(ComboInput.TTSInput);
        FATLRequest ATLRequest(ComboInput.ATLInput, false); // if bGrabAudioData == true, provide relevant check for ATL Request: ATLRequest.bSuccessful

        FComboRequest Request(ComboInput.ComboMode, TTSRequest, ATLRequest, ChatRequest);

        HttpRequest = GetRequestsManager()->MakeComboRequest(
            Request,
            FOnDHRequestCompleted::CreateLambda(
                [this](TSharedPtr<FGenericResponse> Response, bool bSuccess)
                {
                    if (bSuccess && Response->bSuccess)
                    {
                        FComboResponse* ComboResponse = static_cast<FComboResponse*>(Response.Get());
                        check(ComboResponse != nullptr);

                        const FString& ComboResponseFilename = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("MetahumanSDK") / TEXT("combo_response.zip"));
                        
                        if (FFileHelper::SaveArrayToFile(ComboResponse->Data, *ComboResponseFilename))
                        {
                            const FString& ExtractDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("MetahumanSDK") / TEXT("Temp"));

                            IFileManager::Get().DeleteDirectory(*ExtractDirectory, false, true);

                            if (FMetahumanSDKZipUtilities::ExtractArchive(ComboResponseFilename, ExtractDirectory, OutError))
                            {
                                const FString& ResponseJsonFilename = ExtractDirectory / TEXT("response.json");
                                const FString& InputWavFilename = ExtractDirectory / TEXT("input.wav");
                                const FString& AnimationOutputJsonFilename = ExtractDirectory / TEXT("output.json");

                                // load archive entries from the disk and convert to corresponding ue4 assets
                                if (IFileManager::Get().FileExists(*ResponseJsonFilename))
                                {
                                    FString ResponseJson;
                                    if (FFileHelper::LoadFileToString(ResponseJson, *ResponseJsonFilename))
                                    {
                                        FChatResponse ChatResponse;
                                        ChatResponse.ParseResponse(MoveTemp(ResponseJson));

                                        if (ChatResponse.bSuccess)
                                        {
                                            OutText = ChatResponse.Reply;
                                            APIManager->CacheChatMessage(ChatResponse.Reply);
                                        }
                                        else
                                        {
                                            OutError += FString::Printf(TEXT("\r\n%s\r\n"), *ChatResponse.Error);
                                        }
                                    }
                                    else
                                    {
                                        OutError += FString::Printf(TEXT("\r\nCan't load file to string: %s\r\n"), *ResponseJsonFilename);
                                    }
                                }

                                // parse sound
                                if (IFileManager::Get().FileExists(*InputWavFilename))
                                {
                                    TArray<uint8> InputWavData;
                                    if (FFileHelper::LoadFileToArray(InputWavData, *InputWavFilename))
                                    {
                                        if (InputWavData.Num() > 0)
                                        {
                                            FString Error;
                                            OutSound = FSoundWaveFactory::CreateSoundWave(InputWavData, Error);
                                            if (!IsValid(OutSound))
                                            {
                                                OutError += FString::Printf(TEXT("\r\nError while creating SoundWave: %s\r\n"), *Error);
                                            }
                                        }
                                        else
                                        {
                                            OutError += FString::Printf(TEXT("\r\nMissing wav data: %s\r\n"), *InputWavFilename);
                                        }
                                    }
                                    else
                                    {
                                        OutError += FString::Printf(TEXT("\r\nCan't load file to array: %s\r\n"), *InputWavFilename);
                                    }
                                }

                                // parse animation
                                if (IFileManager::Get().FileExists(*AnimationOutputJsonFilename))
                                {
                                    FString AnimationOutput;
                                    if (FFileHelper::LoadFileToString(AnimationOutput, *AnimationOutputJsonFilename))
                                    {
                                        FATLResponse ATLResponse;
                                        ATLResponse.ParseResponse(MoveTemp(AnimationOutput));

                                        if (ATLResponse.bSuccess)
                                        {
                                            OutAnimation = FAnimSequenceFactory::CreateAnimSequence(
                                                ComboInput.ATLInput.Skeleton,
                                                ComboInput.ATLInput.BlendShapeCurveInterpMode,
                                                ComboInput.ATLInput.MappingsInfo,
                                                &ATLResponse, 
                                                OutError
                                            );

                                            if (!IsValid(OutAnimation))
                                            {
                                                OutError += FString::Printf(TEXT("\r\nError while creating SoundWave: %s\r\n"), *OutError);
                                            }
                                        }
                                        else
                                        {
                                            OutError += FString::Printf(TEXT("\r\n%s\r\n"), *ATLResponse.Error);
                                        }
                                    }
                                    else
                                    {
                                        OutError += FString::Printf(TEXT("\r\nCan't load file to string: %s\r\n"), *AnimationOutputJsonFilename);
                                    }
                                }
                            }
                            else
                            {
                                OutError = FString::Printf(TEXT("Can't extract archive %s to directory: %s"), *ComboResponseFilename, *ExtractDirectory);
                            }
                        }
                        else
                        {
                            OutError = FString::Printf(TEXT("Can't save combo response to filename: %s"), *ComboResponseFilename);
                        }
                    }
                    else
                    {
                        OutError = bSuccess ? Response->Error : TEXT("");
                    }
                    bRequestCompleted = true;
                }
            )
        );

        bRequestSent = true;

        const double EndTime = FPlatformTime::Seconds();
        UE_LOG(LogMetahumanSDKComboAction, Log, TEXT("Combo request took %f seconds to be sent"), EndTime - StartTime);
    }
}

void FComboAction::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(OutSound);
    Collector.AddReferencedObject(OutAnimation);
}

bool FComboAction::IsActionCompleted() const
{
    return bRequestSent && bRequestCompleted;
}

bool FComboAction::IsSuccess() const
{
    if (ComboInput.ComboMode == EComboMode::ECHAT_TTS_ATL)
    {
        return IsActionCompleted() && IsValid(OutSound) && IsValid(OutAnimation) && OutText.Len() > 0 && OutError.Len() == 0;
    }
    else if (ComboInput.ComboMode == EComboMode::ECHAT_TTS)
    {
        return IsActionCompleted() && IsValid(OutSound) && OutText.Len() > 0 && OutError.Len() == 0;
    }
    
    // TTS_ATL
    return IsActionCompleted() && IsValid(OutSound) && IsValid(OutAnimation) && OutError.Len() == 0;
}

void FComboAction::NotifyObjectDestroyed()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}

void FComboAction::NotifyActionAborted()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}