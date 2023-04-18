#pragma once

#include "ATLStreamAction.h"
#include "ATLStreamBuffer.h"
#include "MetahumanSDKAPIInput.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "BaseStreamAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBaseStreamActionOutputPin,UAnimSequence*,OutAnimation,UATLStreamBuffer*,Buffer,bool,bSuccess,FString,Error);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FATLStreamActionOutputPin,UATLStreamBuffer*,Buffer);

UCLASS()
class UBaseStreamAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FBaseStreamActionOutputPin Chunk;

	UPROPERTY(BlueprintAssignable)
	FBaseStreamActionOutputPin Completed;

	UPROPERTY(BlueprintAssignable)
	FBaseStreamActionOutputPin Started;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "AsyncNode")
	static UBaseStreamAction* ATLStream(FMetahumanSDKATLInput Input);

	virtual void Activate() override;

	virtual void BeginDestroy() override;
	
private:
	/**
	* Static property to prevent restarting the async node multiple times before execution has finished
	*/
	static bool bActive;

	FMetahumanSDKATLInput ATLInput;

	FATLStreamAction* ATLStreamAction;
};
