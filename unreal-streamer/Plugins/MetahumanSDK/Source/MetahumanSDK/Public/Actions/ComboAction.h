#pragma once

#include "CoreMinimal.h"
#include "BasePendingAction.h"
#include "Interfaces/IHttpRequest.h"


class FComboAction : public FBasePendingAction
{

public:
    FComboAction(const FMetahumanSDKComboInput& InComboInput);
    ~FComboAction();

    void Initialize() override;
    void Execute() override;

#if ENGINE_MAJOR_VERSION == 5
    virtual FString GetReferencerName() const override {return TEXT("FComboAction");};
#endif
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual void NotifyObjectDestroyed() override;
    virtual void NotifyActionAborted() override;

    bool IsActionCompleted() const override;
    bool IsSuccess() const override;

    class USoundWave* OutSound = nullptr;
    class UAnimSequence* OutAnimation = nullptr;
    FString OutText = "";
    FString OutError = "";

private:
    FMetahumanSDKComboInput ComboInput;
    
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
    TSharedPtr<IHttpRequest> HttpRequest;
#else
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest;
#endif
    
    bool bRequestSent = false;
    bool bRequestCompleted = false;
};
