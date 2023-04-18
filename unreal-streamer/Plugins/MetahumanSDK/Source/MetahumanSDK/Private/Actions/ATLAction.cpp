#include "ATLAction.h"
#include "Animation/AnimSequence.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKRequestManager.h"
#include "MetahumanSDKResponses.h"
#include "AnimSequenceFactory.h"


DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKATLAction, Log, All);


FATLAction::FATLAction(const FMetahumanSDKATLInput& InATLInput)
: ATLInput(InATLInput), bRequestSent(false), bRequestCompleted(false)
{
    Initialize();
}

FATLAction::~FATLAction()
{

}

void FATLAction::Initialize()
{
    bRequestSent = false;
    bRequestCompleted = false;
    HttpRequest = nullptr;
}

void FATLAction::Execute()
{
    check(GetRequestsManager().IsValid());

    if (!bRequestSent)
    {
        const FATLRequest ATLRequest(ATLInput, IsValid(ATLInput.Sound));
        if (!ATLRequest.bSuccessful)
        {
            OutAnimation = nullptr;
            OutError = "Can't prepare ATL request with provided sound wave!";
            bRequestCompleted = true;

            return;
        }
        
        const double StartTime = FPlatformTime::Seconds();              

        HttpRequest = GetRequestsManager()->MakeATLRequest(
            ATLRequest,
            FOnDHRequestCompleted::CreateLambda(
                [this](TSharedPtr<FGenericResponse> Response, bool bSuccess)
                {
                    if (bSuccess && Response->bSuccess)
                    {
                        FATLResponse* ATLResponse = static_cast<FATLResponse*>(Response.Get());
                        check(ATLResponse != nullptr);
                
                        OutAnimation = FAnimSequenceFactory::CreateAnimSequence(ATLInput.Skeleton, ATLInput.BlendShapeCurveInterpMode, ATLInput.MappingsInfo, ATLResponse, OutError);
                    }
                    else
                    {
                        OutAnimation = nullptr;
                        OutError = bSuccess ? Response->Error : TEXT("");
                    }
                    
                    bRequestCompleted = true;
                }
            )
        );
        
        bRequestSent = true;

        const double EndTime = FPlatformTime::Seconds();
        UE_LOG(LogMetahumanSDKATLAction, Log, TEXT("ATL request took %f seconds to be sent"), EndTime - StartTime);
    }
}

void FATLAction::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(OutAnimation);
}

void FATLAction::NotifyObjectDestroyed()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}

void FATLAction::NotifyActionAborted()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}

bool FATLAction::IsActionCompleted() const
{
    //return bRequestSent && bRequestCompleted;
    return bRequestCompleted;
}

bool FATLAction::IsSuccess() const
{
    return IsActionCompleted() && IsValid(OutAnimation) && OutError.Len() == 0;
}
