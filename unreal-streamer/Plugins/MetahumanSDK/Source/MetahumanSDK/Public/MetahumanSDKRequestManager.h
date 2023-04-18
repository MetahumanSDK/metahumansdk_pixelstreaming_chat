#pragma once

#include "CoreMinimal.h"
#include "ATLStreamBuffer.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Tickable.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "MetahumanSDKRequests.h"
#include "MetahumanSDKResponses.h"

DECLARE_DELEGATE_TwoParams(FOnDHRequestCompleted, TSharedPtr<FGenericResponse>, bool);


class UMetahumanSDKSettings;
class UMetahumanSDKAPIManager;

class FMetahumanSDKRequestManager : public FRunnable, public FTickableGameObject
{

public:
    FMetahumanSDKRequestManager(UMetahumanSDKAPIManager* InAPIManager);
    ~FMetahumanSDKRequestManager();

public:
    void Tick(float DeltaTime) override;
    TStatId GetStatId() const override;

public:
    void Initialize();
    void Cleanup();

public:
    
    static FString PrevType;
    static bool PrevComboAtl;
    static bool PrevComboChat;
    static bool PrevComboTts;
    static int PrevSize;
    static double PrevTime;
    static double PrevAudioLength;
    
    /* FRunnable interface */
    bool Init() override;
    uint32 Run() override;
    void Exit() override;

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
    TSharedPtr<IHttpRequest> MakeATLRequest(const FATLRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest> MakeATLStreamRequest(const FATLRequest& Request, FOnDHRequestCompleted ChunkRecvDelegate = nullptr, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest> MakeTTSRequest(const FTTSRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest> MakeChatRequest(const FChatRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest> MakeComboRequest(const FComboRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
#else
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> MakeATLRequest(const FATLRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> MakeATLStreamRequest(const FATLRequest& Request, FOnDHRequestCompleted ChunkRecvDelegate = nullptr, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> MakeTTSRequest(const FTTSRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> MakeChatRequest(const FChatRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> MakeComboRequest(const FComboRequest& Request, FOnDHRequestCompleted CompletedDelegate = nullptr);
#endif

    void Trigger();
    void BlockTillAllRequestsFinished();
private:
    
    FString GenerateRequestJson(const FGenericRequest& Request) const;
    bool GenerateAPIToken(FString& OutAPIToken);

private:
    FRunnableThread* WorkerThread = nullptr;
    FEvent* WorkerThreadSemaphore = nullptr;

    FThreadSafeBool bCompletedWork = true; 

    bool bStopThread = false;
    
    int ChunkNum = 0;
    int32 LastProgressUpdateBytes = 0;
private:
    UMetahumanSDKAPIManager* APIManager;
    TWeakObjectPtr<UMetahumanSDKSettings> MetahumanSDKSettings;
};
