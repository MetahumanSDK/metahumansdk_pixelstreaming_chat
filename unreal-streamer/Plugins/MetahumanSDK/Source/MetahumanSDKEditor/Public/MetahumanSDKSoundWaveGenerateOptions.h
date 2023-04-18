#pragma once

#include "CoreMinimal.h"
#include "MetahumanSDKAPIInput.h"
#include "MetahumanSDKSoundWaveGenerateOptions.generated.h"

UCLASS()
class METAHUMANSDKEDITOR_API UMetahumanSDKSoundWaveGenerateOptions : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = GenerateOptions)
    ETTSEngine TTSEngine = ETTSEngine::TTSEngineGoogle;
    
    UPROPERTY(EditAnywhere, Category = GenerateOptions, meta = (EditConditionHides, HideEditConditionToggle, EditCondition = "TTSEngine == ETTSEngine::TTSEngineGoogle"))
    ETTSVoiceGoogle TTSVoiceGoogle = ETTSVoiceGoogle::TTSVoiceGoogleenUSNeural2A;
    
    UPROPERTY(EditAnywhere, Category = GenerateOptions, meta = (EditConditionHides, HideEditConditionToggle, EditCondition = "TTSEngine == ETTSEngine::TTSEngineAzure"))
    ETTSVoiceAzure TTSVoiceAzure = ETTSVoiceAzure::TTSVoiceAzureenUSGuyNeural;
        
    UPROPERTY(EditAnywhere, Category = GenerateOptions, meta=(MultiLine=true))
    FString Text;
};