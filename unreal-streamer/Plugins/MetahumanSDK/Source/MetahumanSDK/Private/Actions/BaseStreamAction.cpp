#include "BaseStreamAction.h"

#include "ATLStreamAction.h"
#include "MetahumanSDKAPIManager.h"

bool UBaseStreamAction::bActive=false; //Init bactive for all instances

UBaseStreamAction* UBaseStreamAction::ATLStream(FMetahumanSDKATLInput Input)
{
	UBaseStreamAction* Node = NewObject<UBaseStreamAction>();
	check(Node != nullptr);

	Node->ATLInput=Input;
	
	return Node;
}

void UBaseStreamAction::Activate()
{
	if (UBaseStreamAction::bActive)
	{
		FFrame::KismetExecutionMessage(TEXT("Async action is already running"), ELogVerbosity::Warning);
		//return;
	}

	FFrame::KismetExecutionMessage(TEXT("UBaseStreamAction Activate!"), ELogVerbosity::Log);
	ATLStreamAction = new FATLStreamAction(ATLInput);

	ATLStreamAction->OnChunk.AddLambda(
		[this](FBasePendingAction* Action, FATLResponse* Response)
		{
			FATLStreamAction* CurrentAction = static_cast<FATLStreamAction*>(Action);
			check(CurrentAction != nullptr);

			Chunk.Broadcast(CurrentAction->OutAnimation, nullptr, true, "");
		});
	
	ATLStreamAction->OnActionCompleted.AddLambda(
		[this](FBasePendingAction* Action)
		{
			FATLStreamAction* CurrentAction = static_cast<FATLStreamAction*>(Action);
			check(CurrentAction != nullptr);

			Completed.Broadcast(CurrentAction->OutAnimation, nullptr, !CurrentAction->OutError.IsEmpty(),"");
			UBaseStreamAction::bActive=false;
		}
	);

	ATLStreamAction->OnStarted.AddLambda(
		[this](FBasePendingAction* Action, UATLStreamBuffer* Buffer, const bool bSuccess)
		{
			const FATLStreamAction* CurrentAction = static_cast<FATLStreamAction*>(Action);
		
			
			Started.Broadcast(nullptr, Buffer, bSuccess, CurrentAction->OutError);
		}
	);	

	UBaseStreamAction::bActive=true;
	
	
	if (UWorld* World = GetWorld())
    { 
    	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

    	FLatentActionInfo LatentInfo;
    	LatentInfo.ExecutionFunction = NAME_None;
    	
    	if (LatentActionManager.FindExistingAction<FATLStreamAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
    	{
    		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, ATLStreamAction);
    	}
    } else
    {
    	ATLStreamAction->Execute();
    }
}

void UBaseStreamAction::BeginDestroy()
{
	Super::BeginDestroy();

	if (ATLStreamAction != nullptr)
	{
		ATLStreamAction->NotifyObjectDestroyed();
	}		
}
