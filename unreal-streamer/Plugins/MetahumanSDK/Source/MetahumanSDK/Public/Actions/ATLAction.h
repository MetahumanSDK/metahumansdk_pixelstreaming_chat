#pragma once

#include "CoreMinimal.h"
#include "BasePendingAction.h"
#include "Interfaces/IHttpRequest.h"


class USoundWave;

class FATLAction : public FBasePendingAction
{

public:
    FATLAction(const FMetahumanSDKATLInput& InATLInput);
    ~FATLAction();

    void Initialize() override;
    void Execute() override;

#if ENGINE_MAJOR_VERSION == 5
    virtual FString GetReferencerName() const override {return TEXT("FATLAction");};
#endif
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual void NotifyObjectDestroyed() override;
    virtual void NotifyActionAborted() override;

    bool IsActionCompleted() const override;
    bool IsSuccess() const override;
    
    class UAnimSequence* OutAnimation = nullptr;
    FString OutError = "";

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
