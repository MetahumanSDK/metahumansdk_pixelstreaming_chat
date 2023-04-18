#pragma once

#include "CoreMinimal.h"
#include "Subsystems/SubsystemCollection.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Queue.h"
#include "Engine/LatentActionManager.h"
#include "Tickable.h"
#include "MetahumanSDKAPIInput.h"
#include "MetahumanSDKAPIOutput.h"
#include "Actions/BasePendingAction.h"
#include "MetahumanSDKAPIManager.generated.h"


class FMetahumanSDKRequestManager;
class USkeleton;


struct FAssetManagerRequest
{
    FAssetManagerRequest()
        : ActionObject(nullptr), UUID(-1), Action(nullptr)
    { }

    FAssetManagerRequest(UObject* InActionObject, int32 InUUID, FBasePendingAction* InAction)
        : ActionObject(InActionObject), UUID(InUUID), Action(InAction)
    { }

    void Invalidate()
    {
        *this = FAssetManagerRequest();
    }

    bool IsValid() const
    {
        return ActionObject != nullptr && UUID != -1 && Action != nullptr;
    }

    UObject* ActionObject = nullptr;
    int32 UUID = -1;
    FBasePendingAction* Action = nullptr;
};

/**
 * 
 */
UCLASS(BlueprintType)
class METAHUMANSDK_API UMetahumanSDKAPIManager : public UEngineSubsystem, public FTickableGameObject
{    
    GENERATED_BODY()

public:
    void Initialize(FSubsystemCollectionBase& Collection) override;
    void Deinitialize() override;

    // API
    UFUNCTION(BlueprintPure, Category = "MetahumanSDKAPIManager")
    static UMetahumanSDKAPIManager* GetMetahumanSDKAPIManager();

    UFUNCTION(BlueprintCallable, Category = "MetahumanSDKAPIManager", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void ATLAudioToLipsync(const FMetahumanSDKATLInput& ATLInput, bool& bSuccess, FMetahumanSDKATLOutput& ATLOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);

    UFUNCTION(BlueprintCallable, Category = "MetahumanSDKAPIManager", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void ATLStreamAudioToLipsync(const FMetahumanSDKATLInput& ATLInput, bool& bSuccess, FMetahumanSDKATLOutput& ATLOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "MetahumanSDKAPIManager", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void TTSTextToSpeech(const FMetahumanSDKTTSInput& TTSInput, bool& bSuccess, FMetahumanSDKTTSOutput& TTSOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "MetahumanSDKAPIManager", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void ChatBot(const FMetahumanSDKChatInput& ChatInput, bool& bSuccess, FMetahumanSDKChatOutput& ChatOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "MetahumanSDKAPIManager", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void Combo(const FMetahumanSDKComboInput& ComboInput, bool& bSuccess, FMetahumanSDKComboOutput& ComboOutput, FString& Error, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "MetahumanSDKAPIManager")
    void ResetChat();
    
    int32 GetChatID() const;
    const TArray<FString>& GetChatHistory() const;  
    
    void CacheChatMessage(const FString& Message);
    
    UFUNCTION(BlueprintCallable, Category = "MetahumanSDKAPIManager")
    void LoadAnimationFromFile(const FString& AnimationJsonFilename, USkeleton* Skeleton, ERichCurveInterpMode InterpolationMode, UAnimSequence*& OutAnimSequence, bool& bSuccess, FString& Error, const FATLMappingsInfo& Mappings);

    TSharedPtr<FMetahumanSDKRequestManager> GetRequestManager();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRequestCompleted);
    UPROPERTY(BlueprintAssignable, Category = "MetahumanSDKAPIManager")
    FOnRequestCompleted OnRequestCompleted;

private:
    void AddRequest(const FAssetManagerRequest& InRequest);

    void Tick(float DeltaTime) override;
    TStatId GetStatId() const override;

    void Cleanup();

private:
    static TQueue<FAssetManagerRequest>* Requests;
    FAssetManagerRequest ActiveRequest;

private:
    TSharedPtr<FMetahumanSDKRequestManager> RequestManager;

    int32 ChatID = -1;
    TArray<FString> ChatHistory;
};