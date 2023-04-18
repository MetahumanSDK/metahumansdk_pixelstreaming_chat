#pragma once

#include "CoreMinimal.h"
#include "MetahumanSDKSettings.generated.h"

/**
 * Settings for the MetahumanSDK plug-in.
 */
UCLASS(Config=Game, defaultconfig)
class METAHUMANSDK_API UMetahumanSDKSettings : public UObject
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    UMetahumanSDKSettings();

    const FString GetPluginVersion();
private:
    FString PluginVersion = "";
    
public:
    /** MetahumanSDK API URL */
    UPROPERTY(Config, EditAnywhere, Category = "MetahumanSDKSettings")
    FString APIURL = TEXT("https://uep-api-west.metahumansdk.io");

    /** Token for MetahumanSDK API*/
    UPROPERTY(Config, EditAnywhere, Category = "MetahumanSDKSettings")
    FString APIToken;
    
    /** Timeout for ATL request in seconds */
    UPROPERTY(Config, EditAnywhere, Category = "MetahumanSDKSettings")
    float ATLRequestTimeoutSeconds = 60.0f;

    /** Timeout for TTS request in seconds */
    UPROPERTY(Config, EditAnywhere, Category = "MetahumanSDKSettings")
    float TTSRequestTimeoutSeconds = 60.0f;
    
    /** Timeout for Chat request in seconds */
    UPROPERTY(Config, EditAnywhere, Category = "MetahumanSDKSettings")
    float ChatRequestTimeoutSeconds = 60.0f;
    
    /** Timeout for Combo request in seconds */
    UPROPERTY(Config, EditAnywhere, Category = "MetahumanSDKSettings")
    float ComboRequestTimeoutSeconds = 60.0f;
};

#if WITH_EDITOR
namespace MetahumanSDKDevSettings
{
    const static bool bCreateAssetsInEditor = false;
}
#endif
