#include "BasePendingAction.h"
#include "Engine/Engine.h"
#include "MetahumanSDKAPIManager.h"


FBasePendingAction::FBasePendingAction()
{
    APIManager = GEngine->GetEngineSubsystem<UMetahumanSDKAPIManager>();
    LatentInfo.ExecutionFunction = NAME_None;
}

void FBasePendingAction::FinishAndTrigger(FLatentActionInfo InLatentInfo)
{
    LatentInfo = InLatentInfo;
}

void FBasePendingAction::AddReferencedObjects(FReferenceCollector& Collector)
{
    
}

void FBasePendingAction::UpdateOperation(FLatentResponse& Response)
{
    Execute();

    if (IsActionCompleted())
    {
        OnActionCompleted.Broadcast(this);
        
    }
    
    // if we got LatentInfo then we need to finish this action
    if (LatentInfo.ExecutionFunction != NAME_None)
    {
        Response.FinishAndTriggerIf(true, LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget);
    }
    // {
    //     Response.FinishAndTriggerIf(IsActionCompleted() || (!IsValid(GEngine->GetWorld()) || GEngine->GetWorld()->IsEditorWorld()), LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget);
    // }
}

TSharedPtr<FMetahumanSDKRequestManager> FBasePendingAction::GetRequestsManager() const
{
    return GEngine->GetEngineSubsystem<UMetahumanSDKAPIManager>()->GetRequestManager();
}
