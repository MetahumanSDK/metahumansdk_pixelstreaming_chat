#pragma once

#include "CoreMinimal.h"
#include "BasePendingAction.h"
#include "Interfaces/IHttpRequest.h"


class FChatAction : public FBasePendingAction
{

public:
    FChatAction(const FMetahumanSDKChatInput& ChatInput);
    ~FChatAction();

    void Initialize() override;
    void Execute() override;

    bool IsActionCompleted() const override;
    bool IsSuccess() const override;

    virtual void NotifyObjectDestroyed() override;
    virtual void NotifyActionAborted() override;
    
    FString OutText = "";
    FString OutError = "";

private:
    FMetahumanSDKChatInput ChatInput;

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
    TSharedPtr<IHttpRequest> HttpRequest;
#else
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest;
#endif
    
    bool bRequestSent = false;
    bool bRequestCompleted = false;
};
