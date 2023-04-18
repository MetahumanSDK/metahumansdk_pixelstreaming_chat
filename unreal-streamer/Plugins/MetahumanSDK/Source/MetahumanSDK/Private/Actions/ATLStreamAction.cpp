#include "ATLStreamAction.h"
#include "Animation/AnimSequence.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKRequestManager.h"
#include "MetahumanSDKResponses.h"
#include "AnimSequenceFactory.h"


DEFINE_LOG_CATEGORY_STATIC(LogMetahumanSDKATLStreamAction, Log, All);

FATLStreamAction::FATLStreamAction(const FMetahumanSDKATLInput& InATLInput)
: ATLInput(InATLInput), bRequestSent(false), bRequestCompleted(false)
{
    Initialize();
}

FATLStreamAction::~FATLStreamAction()
{
    //UE_LOG(LogTemp, Warning, TEXT("~FATLStreamAction()"));
}

void FATLStreamAction::Initialize()
{
    bRequestSent = false;
    bRequestCompleted = false;
    HttpRequest = nullptr;

    Buffer = NewObject<UATLStreamBuffer>();
}

void FATLStreamAction::Execute()
{
    check(GetRequestsManager().IsValid());

    if (!bRequestSent)
    {
        FATLRequest ATLRequest(ATLInput);        
        if (!ATLRequest.bSuccessful)
        {            
            OutError = "Can't prepare ATL streaming request with provided sound wave!";            
            bRequestCompleted = true;

            if (OnStarted.IsBound())
            {    
                OnStarted.Broadcast(this, Buffer, false);
            }
            
           return;
        }
        
        const double StartTime = FPlatformTime::Seconds();       
       
        Buffer->Clear();
        Buffer->ExpectedLength = ATLInput.Sound->Duration;

        HttpRequest = GetRequestsManager()->MakeATLStreamRequest(
            ATLRequest,
            FOnDHRequestCompleted::CreateLambda(
                [this, StartTime](TSharedPtr<FGenericResponse> Response, bool bSuccess)
                {
                    if (!bSuccess)
                    {
                        return;
                    }
                    
                    const double EndTime = FPlatformTime::Seconds();
                    UE_LOG(LogMetahumanSDKATLStreamAction, Log, TEXT("ATL stream chunk recieved, t = %f seconds"), EndTime - StartTime);

                    FATLResponse* ATLResponse = static_cast<FATLResponse*>(Response.Get());
                    check(ATLResponse != nullptr);

                    if (ATLResponse->bSuccess)
                    {
                        OutAnimation = FAnimSequenceFactory::CreateAnimSequence(
                            ATLInput.Skeleton, ATLInput.BlendShapeCurveInterpMode, ATLInput.MappingsInfo, ATLResponse,
                            OutError);
                    }
                    else
                    {
                        OutAnimation = nullptr;
                    }
                    
                    OutError = Response->Error;

                    if (IsValid(Buffer)) {
                        Buffer->AddChunk(OutAnimation);
                        
                        if (OnChunk.IsBound())
                        {    
                            OnChunk.Broadcast(this, ATLResponse);
                        }
                    }
                }
            ),
            FOnDHRequestCompleted::CreateLambda(
                [this, StartTime](TSharedPtr<FGenericResponse> Response, bool bSuccess)
                {
                    const double EndTime = FPlatformTime::Seconds();
                    UE_LOG(LogMetahumanSDKATLStreamAction, Log, TEXT("ATL stream request complete, t = %f seconds"), EndTime - StartTime);
                    
                    if (bSuccess && Response->bSuccess)
                    {
                        OutError = "";
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

        if (OnStarted.IsBound())
        {    
            OnStarted.Broadcast(this, Buffer, OutError.IsEmpty());
        }

        bRequestSent = true;

        const double EndTime = FPlatformTime::Seconds();
        UE_LOG(LogMetahumanSDKATLStreamAction, Log, TEXT("ATL stream request took %f seconds to be sent"), EndTime - StartTime);
    }
}

void FATLStreamAction::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(OutAnimation);
    Collector.AddReferencedObject(Buffer);
}

bool FATLStreamAction::IsActionCompleted() const
{
    //return bRequestSent && bRequestCompleted;
    return bRequestCompleted;
}

bool FATLStreamAction::IsSuccess() const
{
    return IsActionCompleted() && IsValid(OutAnimation) && OutError.Len() == 0;
}

void FATLStreamAction::NotifyObjectDestroyed()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}

void FATLStreamAction::NotifyActionAborted()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}
