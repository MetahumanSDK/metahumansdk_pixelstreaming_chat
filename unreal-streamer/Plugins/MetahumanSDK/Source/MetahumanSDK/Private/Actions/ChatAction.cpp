#include "ChatAction.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKRequestManager.h"
#include "MetahumanSDKResponses.h"


FChatAction::FChatAction(const FMetahumanSDKChatInput& InChatInput)
: ChatInput(InChatInput), bRequestSent(false), bRequestCompleted(false)
{
    Initialize();
}

FChatAction::~FChatAction()
{

}

void FChatAction::Initialize()
{
    bRequestSent = false;
    bRequestCompleted = false;
    HttpRequest = nullptr;
}

void FChatAction::Execute()
{
    check(GetRequestsManager().IsValid());

    if (!bRequestSent)
    {
        FChatRequest Request(ChatInput);
        {
            APIManager->CacheChatMessage(ChatInput.Text);
            Request.ChatID = APIManager->GetChatID();
            Request.ChatHistory = APIManager->GetChatHistory();
        }

        HttpRequest = GetRequestsManager()->MakeChatRequest(
            Request,
            FOnDHRequestCompleted::CreateLambda(
                [this](TSharedPtr<FGenericResponse> Response, bool bSuccess)
                {   
                    if (bSuccess && Response->bSuccess)
                    {
                        FChatResponse* ChatResponse = static_cast<FChatResponse*>(Response.Get());
                        check (ChatResponse != nullptr);

                        if (ChatResponse->bSuccess)
                        {
                            OutText = ChatResponse->Reply;
                            APIManager->CacheChatMessage(OutText);
                        }
                        else
                        {
                            OutError = ChatResponse->Error;
                        }
                    }
                    else
                    {
                        OutText = "";
                        OutError = bSuccess ? Response->Error : TEXT("");
                    }
                    
                    bRequestCompleted = true;
                }
            )
        );

        bRequestSent = true;
    }
}

bool FChatAction::IsActionCompleted() const
{
    return bRequestSent&& bRequestCompleted;
}

bool FChatAction::IsSuccess() const
{
    return OutText.Len() > 0 && OutError.Len() == 0;
}

void FChatAction::NotifyObjectDestroyed()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}

void FChatAction::NotifyActionAborted()
{
    if (bRequestSent && HttpRequest != nullptr)
    {
        HttpRequest->CancelRequest();
    }
}