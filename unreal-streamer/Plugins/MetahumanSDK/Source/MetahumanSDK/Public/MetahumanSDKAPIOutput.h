#pragma once

#include "CoreMinimal.h"

#include "MetahumanSDKAPIOutput.generated.h"

USTRUCT(BlueprintType)
struct FMetahumanSDKATLOutput
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetahumanSDKATLOutput")
    class UAnimSequence* Animation = nullptr;
};

USTRUCT(BlueprintType)
struct FMetahumanSDKTTSOutput
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetahumanSDKTTSOutput")
    class USoundWave* Sound = nullptr;
};

USTRUCT(BlueprintType)
struct FMetahumanSDKChatOutput
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetahumanSDKChatOutput")
    FString Text;
};

USTRUCT(BlueprintType)
struct FMetahumanSDKComboOutput
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetahumanSDKComboOutput")
    FMetahumanSDKTTSOutput TTSOutput;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetahumanSDKComboOutput")
    FMetahumanSDKATLOutput ATLOutput;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetahumanSDKComboOutput")
    FMetahumanSDKChatOutput ChatOutput;
};