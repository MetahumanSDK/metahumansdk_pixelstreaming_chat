#pragma once

#include "CoreMinimal.h"
#include "BasePendingAction.h"
#include "MetahumanSDKResponses.h"
#include "ATLStreamBuffer.h"
#include "Interfaces/IHttpRequest.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnATLStreamActionChunk, FBasePendingAction*, FATLResponse*);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnATLStreamActionStarted, FBasePendingAction*, UATLStreamBuffer*, bool);

class USoundWave;

class FATLStreamAction : public FBasePendingAction
{

public:
    FATLStreamAction(const FMetahumanSDKATLInput& InATLInput);
    ~FATLStreamAction();

    void Initialize() override;
    void Execute() override;

#if ENGINE_MAJOR_VERSION == 5
    virtual FString GetReferencerName() const override {return TEXT("FATLStreamAction");};
#endif
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual void NotifyObjectDestroyed() override;
    virtual void NotifyActionAborted() override;

    bool IsActionCompleted() const override;
    bool IsSuccess() const override;

    class UAnimSequence* OutAnimation = nullptr;
    FString OutError = "";

    FOnATLStreamActionChunk OnChunk;
    FOnATLStreamActionStarted OnStarted;

    UATLStreamBuffer* Buffer;

    void SetATLInput(const FMetahumanSDKATLInput& AtlInput)
    {
        ATLInput = AtlInput;
    }

private:
    FMetahumanSDKATLInput ATLInput;

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
    TSharedPtr<IHttpRequest> HttpRequest;
#else
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest;
#endif

    bool bRequestSent = false;
    bool bRequestCompleted = false;
};
