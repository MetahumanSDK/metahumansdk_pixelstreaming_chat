#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "UObject/UObjectGlobals.h"
#include "LatentActions.h"
#include "Engine/LatentActionManager.h"
#include "UObject/GCObject.h"
#include "MetahumanSDKAPIInput.h"


class UMetahumanSDKAPIManager;
class FPendingLatentAction;
struct FLatentActionInfo;


class FBasePendingAction;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnActionCompleted, FBasePendingAction*);


class FBasePendingAction : public FPendingLatentAction, public FGCObject
{

public:
    FBasePendingAction();

    virtual void Initialize() {}

    virtual void Execute() = 0;

    void FinishAndTrigger(FLatentActionInfo InLatentInfo);

    virtual bool IsActionCompleted() const = 0;

#if ENGINE_MAJOR_VERSION == 5
    virtual FString GetReferencerName() const override {return TEXT("FBasePendingAction");};
#endif
    
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    virtual bool IsSuccess() const = 0;

public:
    FOnActionCompleted OnActionCompleted;

protected:
    virtual void UpdateOperation(FLatentResponse& Response) override;

protected:
    UMetahumanSDKAPIManager* APIManager;
    TSharedPtr<class FMetahumanSDKRequestManager> GetRequestsManager() const;

private:
    FLatentActionInfo LatentInfo;
};
