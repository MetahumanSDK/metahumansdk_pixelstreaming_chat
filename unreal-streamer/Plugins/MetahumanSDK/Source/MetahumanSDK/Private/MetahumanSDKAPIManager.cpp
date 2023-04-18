#include "MetahumanSDKAPIManager.h"

#include "ATLStreamAction.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Sound/SoundWave.h"
#include "MetahumanSDKRequestManager.h"
#include "MetahumanSDKResponses.h"
#include "MetahumanSDKUtils.h"
#include "MetahumanSDKSettings.h"
#include "Actions/TTSAction.h"
#include "Actions/ATLAction.h"
#include "Actions/ChatAction.h"
#include "Actions/ComboAction.h"
#include "AssetFactories/AnimSequenceFactory.h"


DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKAPIManager, Warning, All);

TQueue<FAssetManagerRequest>* UMetahumanSDKAPIManager::Requests = new TQueue<FAssetManagerRequest>();


UMetahumanSDKAPIManager* UMetahumanSDKAPIManager::GetMetahumanSDKAPIManager()
{
    UMetahumanSDKAPIManager* APIManager = GEngine->GetEngineSubsystem<UMetahumanSDKAPIManager>();
    return APIManager;
}

void UMetahumanSDKAPIManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    {
        RequestManager = MakeShareable(new FMetahumanSDKRequestManager(this));
        RequestManager->Initialize();
    }

    ActiveRequest = FAssetManagerRequest();
    Requests = new TQueue<FAssetManagerRequest>();

    // ResetChat();
}

void UMetahumanSDKAPIManager::Deinitialize()
{
    Cleanup();

    Super::Deinitialize();
}

void UMetahumanSDKAPIManager::ATLAudioToLipsync(const FMetahumanSDKATLInput& ATLInput, bool& bSuccess, FMetahumanSDKATLOutput& ATLOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!IsValid(World))
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("World is not valid!"));
        return;
    }

    if (GetDefault<UMetahumanSDKSettings>()->APIToken.Len() == 0)
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("Fill API token via Project Settings!"));
        return;
    }

    if (!ATLInput.Validate(Error))
    {
        bSuccess = false;
        FLatentActionUtilities::CompleteLatentAction(LatentInfo);
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("ATL request input is not valid! Error: %s"), *Error);
        return;
    }

    // will be deleted by LatentActionManager
    FATLAction* ATLAction = new FATLAction(ATLInput);

    ATLAction->OnActionCompleted.AddLambda(
        [&bSuccess, &ATLOutput, &Error, LatentInfo](FBasePendingAction* Action)
        {
            FATLAction* CurrentAction = static_cast<FATLAction*>(Action);
            check(CurrentAction != nullptr);
           
            bSuccess = CurrentAction->IsSuccess();
            if (bSuccess)
            {
                ATLOutput.Animation = CurrentAction->OutAnimation;
            }
            else
            {
                Error = CurrentAction->OutError;
                UE_LOG(LogMetahumanSDKAPIManager, Error, TEXT("ATL request error: %s"), *Error);
            }

            CurrentAction->FinishAndTrigger(LatentInfo);
        }
    );

    AddRequest(FAssetManagerRequest(LatentInfo.CallbackTarget, LatentInfo.UUID, ATLAction));
}

void UMetahumanSDKAPIManager::TTSTextToSpeech(const FMetahumanSDKTTSInput& TTSInput, bool& bSuccess, FMetahumanSDKTTSOutput& TTSOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!IsValid(World))
    { 
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("World is not valid!"));
        return;
    }

    if (GetDefault<UMetahumanSDKSettings>()->APIToken.Len() == 0)
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("Fill API token via Project Settings!"));
        return;
    }

    if (!TTSInput.Validate(Error))
    {
        bSuccess = false;
        FLatentActionUtilities::CompleteLatentAction(LatentInfo);
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("TTS request input is not valid! Error: %s"), *Error);
        return;
    }

    // will be deleted by LatentActionManager
    FTTSAction* TTSAction = new FTTSAction(TTSInput);

    TTSAction->OnActionCompleted.AddLambda(
        [&bSuccess, &TTSOutput, &Error, LatentInfo](FBasePendingAction* Action)
        {
            FTTSAction* CurrentAction = static_cast<FTTSAction*>(Action);
            check(CurrentAction != nullptr);

            bSuccess = CurrentAction->IsSuccess();
            if (bSuccess)
            {
                TTSOutput.Sound = CurrentAction->OutSound;
            }
            else
            {
                Error = CurrentAction->OutError;
                UE_LOG(LogMetahumanSDKAPIManager, Error, TEXT("TTS request error: %s"), *Error);
            }

            CurrentAction->FinishAndTrigger(LatentInfo);
        }
    );

    AddRequest(FAssetManagerRequest(LatentInfo.CallbackTarget, LatentInfo.UUID, TTSAction));
}

void UMetahumanSDKAPIManager::ATLStreamAudioToLipsync(const FMetahumanSDKATLInput& ATLInput, bool& bSuccess, FMetahumanSDKATLOutput& ATLOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!IsValid(World))
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("World is not valid!"));
        return;
    }

    if (GetDefault<UMetahumanSDKSettings>()->APIToken.Len() == 0)
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("Fill API token via Project Settings!"));
        return;
    }

    if (!ATLInput.Validate(Error))
    {
        bSuccess = false;
        FLatentActionUtilities::CompleteLatentAction(LatentInfo);
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("ATL request input is not valid! Error: %s"), *Error);
        return;
    }

    // will be deleted by LatentActionManager
    FATLStreamAction* ATLStreamAction = new FATLStreamAction(ATLInput);

    ATLStreamAction->OnStarted.AddLambda([&bSuccess](FBasePendingAction* Action, UATLStreamBuffer* Buffer, const bool bSuccessOnStarted)
    {
        const FATLStreamAction* CurrentAction = static_cast<FATLStreamAction*>(Action);
        bSuccess = bSuccessOnStarted;
    });
    
    ATLStreamAction->OnActionCompleted.AddLambda(
        [&bSuccess, &ATLOutput, &Error, LatentInfo](FBasePendingAction* Action)
        {
            FATLStreamAction* CurrentAction = static_cast<FATLStreamAction*>(Action);
            check(CurrentAction != nullptr);
           
            bSuccess = CurrentAction->IsSuccess();
            if (bSuccess)
            {
                ATLOutput.Animation = CurrentAction->OutAnimation;
            }
            else
            {
                Error = CurrentAction->OutError;
                UE_LOG(LogMetahumanSDKAPIManager, Error, TEXT("ATL stream request error: %s"), *Error);
            }

            CurrentAction->FinishAndTrigger(LatentInfo);
        }
    );

    AddRequest(FAssetManagerRequest(LatentInfo.CallbackTarget, LatentInfo.UUID, ATLStreamAction));
}

void UMetahumanSDKAPIManager::ChatBot(const FMetahumanSDKChatInput& ChatInput, bool& bSuccess, FMetahumanSDKChatOutput& ChatOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!IsValid(World))
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("World is not valid!"));
        return;
    }

    if (GetDefault<UMetahumanSDKSettings>()->APIToken.Len() == 0)
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("Fill API token via Project Settings!"));
        return;
    }

    if (!ChatInput.Validate(Error))
    {
        bSuccess = false;
        FLatentActionUtilities::CompleteLatentAction(LatentInfo);
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("Chat request input is not valid! Error: %s"), *Error);
        return;
    }

    // will be deleted by LatentActionManager
    FChatAction* ChatAction = new FChatAction(ChatInput);

    ChatAction->OnActionCompleted.AddLambda(
        [&bSuccess, &ChatOutput, &Error, LatentInfo](FBasePendingAction* Action)
        {
            FChatAction* CurrentAction = static_cast<FChatAction*>(Action);
            check(CurrentAction != nullptr);

            bSuccess = CurrentAction->IsSuccess();
            if (bSuccess)
            {
                ChatOutput.Text = CurrentAction->OutText;
            }
            else
            {
                Error = CurrentAction->OutError;
                UE_LOG(LogMetahumanSDKAPIManager, Error, TEXT("Chat request error: %s"), *Error);
            }

            CurrentAction->FinishAndTrigger(LatentInfo);
        }
    );

    AddRequest(FAssetManagerRequest(LatentInfo.CallbackTarget, LatentInfo.UUID, ChatAction));
}

void UMetahumanSDKAPIManager::Combo(const FMetahumanSDKComboInput& ComboInput, bool& bSuccess, FMetahumanSDKComboOutput& ComboOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!IsValid(World))
    {
        return;
    }

    if (GetDefault<UMetahumanSDKSettings>()->APIToken.Len() == 0)
    {
        bSuccess = false;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("Fill API token via Project Settings!"));
        return;
    }

    if (!ComboInput.Validate(Error))
    {
        bSuccess = false;
        FLatentActionUtilities::CompleteLatentAction(LatentInfo);
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("Combo request input is not valid! Error: %s"), *Error);
        return;
    }

    // will be deleted by LatentActionManager
    FComboAction* ComboAction = new FComboAction(ComboInput);

    ComboAction->OnActionCompleted.AddLambda(
        [&bSuccess, &ComboOutput, &Error, LatentInfo](FBasePendingAction* Action)
        {
            FComboAction* CurrentAction = static_cast<FComboAction*>(Action);
            check(CurrentAction != nullptr);
            
            bSuccess = CurrentAction->IsSuccess();
            if (bSuccess)
            {
                ComboOutput.TTSOutput.Sound = CurrentAction->OutSound;
                ComboOutput.ATLOutput.Animation = CurrentAction->OutAnimation;
                ComboOutput.ChatOutput.Text = CurrentAction->OutText;

                if (IsValid(ComboOutput.TTSOutput.Sound))
                {
                    FMetahumanSDKRequestManager::PrevAudioLength = ComboOutput.TTSOutput.Sound->GetDuration();
                }
            }
            else
            {
                Error = CurrentAction->OutError;
                UE_LOG(LogMetahumanSDKAPIManager, Error, TEXT("Combo request error: %s"), *CurrentAction->OutError);
            }

            CurrentAction->FinishAndTrigger(LatentInfo);
        }
    );

    AddRequest(FAssetManagerRequest(LatentInfo.CallbackTarget, LatentInfo.UUID, ComboAction));
}

void UMetahumanSDKAPIManager::ResetChat()
{
    ChatID = FMath::Rand();
    ChatHistory.Empty();
}

int32 UMetahumanSDKAPIManager::GetChatID() const
{
    return ChatID;
}

const TArray<FString>& UMetahumanSDKAPIManager::GetChatHistory() const
{
    return ChatHistory;
}

void UMetahumanSDKAPIManager::CacheChatMessage(const FString& Message)
{
    ChatHistory.Add(Message);
}

void UMetahumanSDKAPIManager::LoadAnimationFromFile(const FString& AnimationJsonFilename, USkeleton* Skeleton, ERichCurveInterpMode InterpolationMode, UAnimSequence*& OutAnimSequence, bool& bSuccess, FString& Error, const FATLMappingsInfo& Mappings)
{
    if (!IsValid(Skeleton))
    {
        Error = FString::Printf(TEXT("Provide valid skeleton!"));
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("%s"), *Error);
        bSuccess = false;
        return;
    }
    
    FString DataString;
    if (!FFileHelper::LoadFileToString(DataString, *AnimationJsonFilename))
    {
        Error = FString::Printf(TEXT("File %s does not exist: %s"), *AnimationJsonFilename);
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("%s"), *Error);
        bSuccess = false;
        return;
    }

    FATLResponse ATLResponse;
    ATLResponse.ParseResponse(MoveTemp(DataString));
    if (!ATLResponse.bSuccess)
    {
        Error = ATLResponse.Error;
        UE_LOG(LogMetahumanSDKAPIManager, Warning, TEXT("%s"), *Error);
        bSuccess = false;
        return;
    }

    OutAnimSequence = FAnimSequenceFactory::CreateAnimSequence(Skeleton, InterpolationMode, Mappings, &ATLResponse, Error);
    bSuccess = IsValid(OutAnimSequence);
}

TSharedPtr<FMetahumanSDKRequestManager> UMetahumanSDKAPIManager::GetRequestManager()
{
    ensure(RequestManager.IsValid());
    return RequestManager;
}

void UMetahumanSDKAPIManager::AddRequest(const FAssetManagerRequest& InRequest)
{
    Requests->Enqueue(InRequest);
}   

void UMetahumanSDKAPIManager::Tick(float DeltaTime)
{
    if (ActiveRequest.IsValid() && IsValid(ActiveRequest.ActionObject) && !ActiveRequest.ActionObject->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
    {
        return;
    }
    
    if (!Requests->IsEmpty())
    {
        Requests->Dequeue(ActiveRequest);

        if (!ActiveRequest.IsValid() || !IsValid(ActiveRequest.ActionObject) || ActiveRequest.ActionObject->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
        {
            return;
        }

        ActiveRequest.Action->OnActionCompleted.AddLambda(
            [this](FBasePendingAction* Action)
            {
                ActiveRequest.Invalidate();
            }
        );

        UWorld* World = GEngine->GetWorldFromContextObjectChecked(ActiveRequest.ActionObject);
        check(IsValid(World));

        World->GetLatentActionManager().AddNewAction(
            ActiveRequest.ActionObject,
            ActiveRequest.UUID,
            ActiveRequest.Action
        );
    }
}

TStatId UMetahumanSDKAPIManager::GetStatId() const
{
    return TStatId();
}

void UMetahumanSDKAPIManager::Cleanup()
{
    ActiveRequest = FAssetManagerRequest();
}
