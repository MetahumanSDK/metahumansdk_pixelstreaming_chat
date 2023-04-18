#pragma once

#include "CoreMinimal.h"
#include "Animation/Skeleton.h"
#include "Curves/RealCurve.h"
#include "Sound/SoundWave.h"
#include "Animation/PoseAsset.h"
#include "MetahumanSDKMappingsAsset.h"
#include "MetahumanSDKBoneMappingAsset.h"
#include "MetahumanSDKAPIInput.generated.h"


class UMetahumanSDKMappingsAsset;
class USoundWave;
class UPoseAsset;

UENUM(BlueprintType)
enum class EExplicitEmotion : uint8
{
    EAngry,
    ECalm,
    ECurious,
    EDisgust,
    EDoubt,
    EDreamy,
    EEmbarrassment,
    EFear,
    EFlirt,
    EHappy,
    EPlayful,
    EPositive,
    EResentment,
    EShame,
    ESorrow,
    EStrict,
    ESupplication,
    ESurprise,
    EWrath
};


UENUM(BlueprintType)
enum class EChatLanguage : uint8
{
    CLdeDE UMETA(DisplayName = "de_DE"),
    CLenAU UMETA(DisplayName = "en_AU"),
    CLenCA UMETA(DisplayName = "en_CA"),
    CLenGB UMETA(DisplayName = "en_GB"),
    CLenIN UMETA(DisplayName = "en_IN"),
    CLenUS UMETA(DisplayName = "en_US"),
    CLfrCA UMETA(DisplayName = "fr_CA"),
    CLfrFR UMETA(DisplayName = "fr_FR"),
    CLitIT UMETA(DisplayName = "it_IT"),
    CLjaJP UMETA(DisplayName = "ja_JP"),
    CLesES UMETA(DisplayName = "es_ES"),
    CLesMX UMETA(DisplayName = "es_MX"),
    CLkoKR UMETA(DisplayName = "ko_KR"),
    CLptBR UMETA(DisplayName = "pt_BR"),
};

UENUM(BlueprintType)
enum class ETTSEngine : uint8
{
    TTSEngineGoogle UMETA(DisplayName = "google"),
    TTSEngineAzure UMETA(DisplayName = "azure"),
};

UENUM(BlueprintType)
enum class ETTSVoiceGoogle : uint8
{
    TTSVoiceGoogleafZAStandardA UMETA(DisplayName ="af-ZA-Standard-A"),
    TTSVoiceGooglearXAStandardA UMETA(DisplayName ="ar-XA-Standard-A"),
    TTSVoiceGooglearXAStandardB UMETA(DisplayName ="ar-XA-Standard-B"),
    TTSVoiceGooglearXAStandardC UMETA(DisplayName ="ar-XA-Standard-C"),
    TTSVoiceGooglearXAWavenetA UMETA(DisplayName ="ar-XA-Wavenet-A"),
    TTSVoiceGooglearXAWavenetB UMETA(DisplayName ="ar-XA-Wavenet-B"),
    TTSVoiceGooglearXAWavenetC UMETA(DisplayName ="ar-XA-Wavenet-C"),
    TTSVoiceGooglebnINStandardA UMETA(DisplayName ="bn-IN-Standard-A"),
    TTSVoiceGooglebnINStandardB UMETA(DisplayName ="bn-IN-Standard-B"),
    TTSVoiceGooglebnINWavenetA UMETA(DisplayName ="bn-IN-Wavenet-A"),
    TTSVoiceGooglebnINWavenetB UMETA(DisplayName ="bn-IN-Wavenet-B"),
    TTSVoiceGooglebgbgStandardA UMETA(DisplayName ="bg-bg-Standard-A"),
    TTSVoiceGooglecaesStandardA UMETA(DisplayName ="ca-es-Standard-A"),
    TTSVoiceGoogleyueHKStandardA UMETA(DisplayName ="yue-HK-Standard-A"),
    TTSVoiceGoogleyueHKStandardB UMETA(DisplayName ="yue-HK-Standard-B"),
    TTSVoiceGoogleyueHKStandardC UMETA(DisplayName ="yue-HK-Standard-C"),
    TTSVoiceGooglecsCZStandardA UMETA(DisplayName ="cs-CZ-Standard-A"),
    TTSVoiceGooglecsCZWavenetA UMETA(DisplayName ="cs-CZ-Wavenet-A"),
    TTSVoiceGoogledaDKStandardA UMETA(DisplayName ="da-DK-Standard-A"),
    TTSVoiceGoogledaDKStandardC UMETA(DisplayName ="da-DK-Standard-C"),
    TTSVoiceGoogledaDKWavenetA UMETA(DisplayName ="da-DK-Wavenet-A"),
    TTSVoiceGoogledaDKWavenetC UMETA(DisplayName ="da-DK-Wavenet-C"),
    TTSVoiceGooglenlBEStandardA UMETA(DisplayName ="nl-BE-Standard-A"),
    TTSVoiceGooglenlBEStandardB UMETA(DisplayName ="nl-BE-Standard-B"),
    TTSVoiceGooglenlBEWavenetA UMETA(DisplayName ="nl-BE-Wavenet-A"),
    TTSVoiceGooglenlBEWavenetB UMETA(DisplayName ="nl-BE-Wavenet-B"),
    TTSVoiceGooglenlNLStandardA UMETA(DisplayName ="nl-NL-Standard-A"),
    TTSVoiceGooglenlNLStandardB UMETA(DisplayName ="nl-NL-Standard-B"),
    TTSVoiceGooglenlNLStandardC UMETA(DisplayName ="nl-NL-Standard-C"),
    TTSVoiceGooglenlNLWavenetA UMETA(DisplayName ="nl-NL-Wavenet-A"),
    TTSVoiceGooglenlNLWavenetB UMETA(DisplayName ="nl-NL-Wavenet-B"),
    TTSVoiceGooglenlNLWavenetC UMETA(DisplayName ="nl-NL-Wavenet-C"),
    TTSVoiceGoogleenAUNeural2A UMETA(DisplayName ="en-AU-Neural2-A"),
    TTSVoiceGoogleenAUNeural2C UMETA(DisplayName ="en-AU-Neural2-C"),
    TTSVoiceGoogleenAUStandardA UMETA(DisplayName ="en-AU-Standard-A"),
    TTSVoiceGoogleenAUStandardB UMETA(DisplayName ="en-AU-Standard-B"),
    TTSVoiceGoogleenAUStandardC UMETA(DisplayName ="en-AU-Standard-C"),
    TTSVoiceGoogleenAUWavenetA UMETA(DisplayName ="en-AU-Wavenet-A"),
    TTSVoiceGoogleenAUWavenetB UMETA(DisplayName ="en-AU-Wavenet-B"),
    TTSVoiceGoogleenAUWavenetC UMETA(DisplayName ="en-AU-Wavenet-C"),
    TTSVoiceGoogleenINStandardA UMETA(DisplayName ="en-IN-Standard-A"),
    TTSVoiceGoogleenINStandardB UMETA(DisplayName ="en-IN-Standard-B"),
    TTSVoiceGoogleenINStandardC UMETA(DisplayName ="en-IN-Standard-C"),
    TTSVoiceGoogleenINWavenetA UMETA(DisplayName ="en-IN-Wavenet-A"),
    TTSVoiceGoogleenINWavenetB UMETA(DisplayName ="en-IN-Wavenet-B"),
    TTSVoiceGoogleenINWavenetC UMETA(DisplayName ="en-IN-Wavenet-C"),
    TTSVoiceGoogleenGBNeural2A UMETA(DisplayName ="en-GB-Neural2-A"),
    TTSVoiceGoogleenGBNeural2C UMETA(DisplayName ="en-GB-Neural2-C"),
    TTSVoiceGoogleenGBStandardA UMETA(DisplayName ="en-GB-Standard-A"),
    TTSVoiceGoogleenGBStandardB UMETA(DisplayName ="en-GB-Standard-B"),
    TTSVoiceGoogleenGBStandardC UMETA(DisplayName ="en-GB-Standard-C"),
    TTSVoiceGoogleenGBWavenetA UMETA(DisplayName ="en-GB-Wavenet-A"),
    TTSVoiceGoogleenGBWavenetB UMETA(DisplayName ="en-GB-Wavenet-B"),
    TTSVoiceGoogleenGBWavenetC UMETA(DisplayName ="en-GB-Wavenet-C"),
    TTSVoiceGoogleenUSNeural2A UMETA(DisplayName ="en-US-Neural2-A"),
    TTSVoiceGoogleenUSStandardA UMETA(DisplayName ="en-US-Standard-A"),
    TTSVoiceGoogleenUSStandardB UMETA(DisplayName ="en-US-Standard-B"),
    TTSVoiceGoogleenUSStandardC UMETA(DisplayName ="en-US-Standard-C"),
    TTSVoiceGoogleenUSStandardG UMETA(DisplayName ="en-US-Standard-G"),
    TTSVoiceGoogleenUSStandardH UMETA(DisplayName ="en-US-Standard-H"),
    TTSVoiceGoogleenUSStandardI UMETA(DisplayName ="en-US-Standard-I"),
    TTSVoiceGoogleenUSStandardJ UMETA(DisplayName ="en-US-Standard-J"),
    TTSVoiceGoogleenUSWavenetA UMETA(DisplayName ="en-US-Wavenet-A"),
    TTSVoiceGoogleenUSWavenetB UMETA(DisplayName ="en-US-Wavenet-B"),
    TTSVoiceGoogleenUSWavenetC UMETA(DisplayName ="en-US-Wavenet-C"),
    TTSVoiceGoogleenUSWavenetG UMETA(DisplayName ="en-US-Wavenet-G"),
    TTSVoiceGoogleenUSWavenetH UMETA(DisplayName ="en-US-Wavenet-H"),
    TTSVoiceGoogleenUSWavenetI UMETA(DisplayName ="en-US-Wavenet-I"),
    TTSVoiceGoogleenUSWavenetJ UMETA(DisplayName ="en-US-Wavenet-J"),
    TTSVoiceGooglefilPHStandardA UMETA(DisplayName ="fil-PH-Standard-A"),
    TTSVoiceGooglefilPHStandardB UMETA(DisplayName ="fil-PH-Standard-B"),
    TTSVoiceGooglefilPHStandardC UMETA(DisplayName ="fil-PH-Standard-C"),
    TTSVoiceGooglefilPHWavenetA UMETA(DisplayName ="fil-PH-Wavenet-A"),
    TTSVoiceGooglefilPHWavenetB UMETA(DisplayName ="fil-PH-Wavenet-B"),
    TTSVoiceGooglefilPHWavenetC UMETA(DisplayName ="fil-PH-Wavenet-C"),
    TTSVoiceGooglefiFIStandardA UMETA(DisplayName ="fi-FI-Standard-A"),
    TTSVoiceGooglefiFIWavenetA UMETA(DisplayName ="fi-FI-Wavenet-A"),
    TTSVoiceGooglefrCAStandardA UMETA(DisplayName ="fr-CA-Standard-A"),
    TTSVoiceGooglefrCAStandardB UMETA(DisplayName ="fr-CA-Standard-B"),
    TTSVoiceGooglefrCAStandardC UMETA(DisplayName ="fr-CA-Standard-C"),
    TTSVoiceGooglefrCAWavenetA UMETA(DisplayName ="fr-CA-Wavenet-A"),
    TTSVoiceGooglefrCAWavenetB UMETA(DisplayName ="fr-CA-Wavenet-B"),
    TTSVoiceGooglefrCAWavenetC UMETA(DisplayName ="fr-CA-Wavenet-C"),
    TTSVoiceGooglefrFRStandardA UMETA(DisplayName ="fr-FR-Standard-A"),
    TTSVoiceGooglefrFRStandardB UMETA(DisplayName ="fr-FR-Standard-B"),
    TTSVoiceGooglefrFRStandardC UMETA(DisplayName ="fr-FR-Standard-C"),
    TTSVoiceGooglefrFRWavenetA UMETA(DisplayName ="fr-FR-Wavenet-A"),
    TTSVoiceGooglefrFRWavenetB UMETA(DisplayName ="fr-FR-Wavenet-B"),
    TTSVoiceGooglefrFRWavenetC UMETA(DisplayName ="fr-FR-Wavenet-C"),
    TTSVoiceGoogledeDEStandardA UMETA(DisplayName ="de-DE-Standard-A"),
    TTSVoiceGoogledeDEStandardB UMETA(DisplayName ="de-DE-Standard-B"),
    TTSVoiceGoogledeDEStandardC UMETA(DisplayName ="de-DE-Standard-C"),
    TTSVoiceGoogledeDEWavenetA UMETA(DisplayName ="de-DE-Wavenet-A"),
    TTSVoiceGoogledeDEWavenetB UMETA(DisplayName ="de-DE-Wavenet-B"),
    TTSVoiceGoogledeDEWavenetC UMETA(DisplayName ="de-DE-Wavenet-C"),
    TTSVoiceGoogleelGRStandardA UMETA(DisplayName ="el-GR-Standard-A"),
    TTSVoiceGoogleelGRWavenetA UMETA(DisplayName ="el-GR-Wavenet-A"),
    TTSVoiceGoogleguINStandardA UMETA(DisplayName ="gu-IN-Standard-A"),
    TTSVoiceGoogleguINStandardB UMETA(DisplayName ="gu-IN-Standard-B"),
    TTSVoiceGoogleguINWavenetA UMETA(DisplayName ="gu-IN-Wavenet-A"),
    TTSVoiceGoogleguINWavenetB UMETA(DisplayName ="gu-IN-Wavenet-B"),
    TTSVoiceGooglehiINStandardA UMETA(DisplayName ="hi-IN-Standard-A"),
    TTSVoiceGooglehiINStandardB UMETA(DisplayName ="hi-IN-Standard-B"),
    TTSVoiceGooglehiINStandardC UMETA(DisplayName ="hi-IN-Standard-C"),
    TTSVoiceGooglehiINWavenetA UMETA(DisplayName ="hi-IN-Wavenet-A"),
    TTSVoiceGooglehiINWavenetB UMETA(DisplayName ="hi-IN-Wavenet-B"),
    TTSVoiceGooglehiINWavenetC UMETA(DisplayName ="hi-IN-Wavenet-C"),
    TTSVoiceGooglehuHUStandardA UMETA(DisplayName ="hu-HU-Standard-A"),
    TTSVoiceGooglehuHUWavenetA UMETA(DisplayName ="hu-HU-Wavenet-A"),
    TTSVoiceGoogleisisStandardA UMETA(DisplayName ="is-is-Standard-A"),
    TTSVoiceGoogleidIDStandardA UMETA(DisplayName ="id-ID-Standard-A"),
    TTSVoiceGoogleidIDStandardB UMETA(DisplayName ="id-ID-Standard-B"),
    TTSVoiceGoogleidIDStandardC UMETA(DisplayName ="id-ID-Standard-C"),
    TTSVoiceGoogleidIDWavenetA UMETA(DisplayName ="id-ID-Wavenet-A"),
    TTSVoiceGoogleidIDWavenetB UMETA(DisplayName ="id-ID-Wavenet-B"),
    TTSVoiceGoogleidIDWavenetC UMETA(DisplayName ="id-ID-Wavenet-C"),
    TTSVoiceGoogleitITStandardA UMETA(DisplayName ="it-IT-Standard-A"),
    TTSVoiceGoogleitITStandardB UMETA(DisplayName ="it-IT-Standard-B"),
    TTSVoiceGoogleitITStandardC UMETA(DisplayName ="it-IT-Standard-C"),
    TTSVoiceGoogleitITWavenetA UMETA(DisplayName ="it-IT-Wavenet-A"),
    TTSVoiceGoogleitITWavenetB UMETA(DisplayName ="it-IT-Wavenet-B"),
    TTSVoiceGoogleitITWavenetC UMETA(DisplayName ="it-IT-Wavenet-C"),
    TTSVoiceGooglejaJPStandardA UMETA(DisplayName ="ja-JP-Standard-A"),
    TTSVoiceGooglejaJPStandardB UMETA(DisplayName ="ja-JP-Standard-B"),
    TTSVoiceGooglejaJPStandardC UMETA(DisplayName ="ja-JP-Standard-C"),
    TTSVoiceGooglejaJPWavenetA UMETA(DisplayName ="ja-JP-Wavenet-A"),
    TTSVoiceGooglejaJPWavenetB UMETA(DisplayName ="ja-JP-Wavenet-B"),
    TTSVoiceGooglejaJPWavenetC UMETA(DisplayName ="ja-JP-Wavenet-C"),
    TTSVoiceGoogleknINStandardA UMETA(DisplayName ="kn-IN-Standard-A"),
    TTSVoiceGoogleknINStandardB UMETA(DisplayName ="kn-IN-Standard-B"),
    TTSVoiceGoogleknINWavenetA UMETA(DisplayName ="kn-IN-Wavenet-A"),
    TTSVoiceGoogleknINWavenetB UMETA(DisplayName ="kn-IN-Wavenet-B"),
    TTSVoiceGooglekoKRStandardA UMETA(DisplayName ="ko-KR-Standard-A"),
    TTSVoiceGooglekoKRStandardB UMETA(DisplayName ="ko-KR-Standard-B"),
    TTSVoiceGooglekoKRStandardC UMETA(DisplayName ="ko-KR-Standard-C"),
    TTSVoiceGooglekoKRWavenetA UMETA(DisplayName ="ko-KR-Wavenet-A"),
    TTSVoiceGooglekoKRWavenetB UMETA(DisplayName ="ko-KR-Wavenet-B"),
    TTSVoiceGooglekoKRWavenetC UMETA(DisplayName ="ko-KR-Wavenet-C"),
    TTSVoiceGooglelvlvStandardA UMETA(DisplayName ="lv-lv-Standard-A"),
    TTSVoiceGooglemsMYStandardA UMETA(DisplayName ="ms-MY-Standard-A"),
    TTSVoiceGooglemsMYStandardB UMETA(DisplayName ="ms-MY-Standard-B"),
    TTSVoiceGooglemsMYStandardC UMETA(DisplayName ="ms-MY-Standard-C"),
    TTSVoiceGooglemsMYWavenetA UMETA(DisplayName ="ms-MY-Wavenet-A"),
    TTSVoiceGooglemsMYWavenetB UMETA(DisplayName ="ms-MY-Wavenet-B"),
    TTSVoiceGooglemsMYWavenetC UMETA(DisplayName ="ms-MY-Wavenet-C"),
    TTSVoiceGooglemlINStandardA UMETA(DisplayName ="ml-IN-Standard-A"),
    TTSVoiceGooglemlINStandardB UMETA(DisplayName ="ml-IN-Standard-B"),
    TTSVoiceGooglemlINWavenetA UMETA(DisplayName ="ml-IN-Wavenet-A"),
    TTSVoiceGooglemlINWavenetB UMETA(DisplayName ="ml-IN-Wavenet-B"),
    TTSVoiceGooglecmnCNStandardA UMETA(DisplayName ="cmn-CN-Standard-A"),
    TTSVoiceGooglecmnCNStandardB UMETA(DisplayName ="cmn-CN-Standard-B"),
    TTSVoiceGooglecmnCNStandardC UMETA(DisplayName ="cmn-CN-Standard-C"),
    TTSVoiceGooglecmnCNWavenetA UMETA(DisplayName ="cmn-CN-Wavenet-A"),
    TTSVoiceGooglecmnCNWavenetB UMETA(DisplayName ="cmn-CN-Wavenet-B"),
    TTSVoiceGooglecmnCNWavenetC UMETA(DisplayName ="cmn-CN-Wavenet-C"),
    TTSVoiceGooglecmnTWStandardA UMETA(DisplayName ="cmn-TW-Standard-A"),
    TTSVoiceGooglecmnTWStandardB UMETA(DisplayName ="cmn-TW-Standard-B"),
    TTSVoiceGooglecmnTWStandardC UMETA(DisplayName ="cmn-TW-Standard-C"),
    TTSVoiceGooglecmnTWWavenetA UMETA(DisplayName ="cmn-TW-Wavenet-A"),
    TTSVoiceGooglecmnTWWavenetB UMETA(DisplayName ="cmn-TW-Wavenet-B"),
    TTSVoiceGooglecmnTWWavenetC UMETA(DisplayName ="cmn-TW-Wavenet-C"),
    TTSVoiceGooglenbNOStandardA UMETA(DisplayName ="nb-NO-Standard-A"),
    TTSVoiceGooglenbNOStandardB UMETA(DisplayName ="nb-NO-Standard-B"),
    TTSVoiceGooglenbNOStandardC UMETA(DisplayName ="nb-NO-Standard-C"),
    TTSVoiceGooglenbNOWavenetA UMETA(DisplayName ="nb-NO-Wavenet-A"),
    TTSVoiceGooglenbNOWavenetB UMETA(DisplayName ="nb-NO-Wavenet-B"),
    TTSVoiceGooglenbNOWavenetC UMETA(DisplayName ="nb-NO-Wavenet-C"),
    TTSVoiceGoogleplPLStandardA UMETA(DisplayName ="pl-PL-Standard-A"),
    TTSVoiceGoogleplPLStandardB UMETA(DisplayName ="pl-PL-Standard-B"),
    TTSVoiceGoogleplPLStandardC UMETA(DisplayName ="pl-PL-Standard-C"),
    TTSVoiceGoogleplPLWavenetA UMETA(DisplayName ="pl-PL-Wavenet-A"),
    TTSVoiceGoogleplPLWavenetB UMETA(DisplayName ="pl-PL-Wavenet-B"),
    TTSVoiceGoogleplPLWavenetC UMETA(DisplayName ="pl-PL-Wavenet-C"),
    TTSVoiceGoogleptBRStandardA UMETA(DisplayName ="pt-BR-Standard-A"),
    TTSVoiceGoogleptBRStandardB UMETA(DisplayName ="pt-BR-Standard-B"),
    TTSVoiceGoogleptBRStandardC UMETA(DisplayName ="pt-BR-Standard-C"),
    TTSVoiceGoogleptBRWavenetA UMETA(DisplayName ="pt-BR-Wavenet-A"),
    TTSVoiceGoogleptBRWavenetB UMETA(DisplayName ="pt-BR-Wavenet-B"),
    TTSVoiceGoogleptBRWavenetC UMETA(DisplayName ="pt-BR-Wavenet-C"),
    TTSVoiceGoogleptPTStandardA UMETA(DisplayName ="pt-PT-Standard-A"),
    TTSVoiceGoogleptPTStandardB UMETA(DisplayName ="pt-PT-Standard-B"),
    TTSVoiceGoogleptPTStandardC UMETA(DisplayName ="pt-PT-Standard-C"),
    TTSVoiceGoogleptPTWavenetA UMETA(DisplayName ="pt-PT-Wavenet-A"),
    TTSVoiceGoogleptPTWavenetB UMETA(DisplayName ="pt-PT-Wavenet-B"),
    TTSVoiceGoogleptPTWavenetC UMETA(DisplayName ="pt-PT-Wavenet-C"),
    TTSVoiceGooglepaINStandardA UMETA(DisplayName ="pa-IN-Standard-A"),
    TTSVoiceGooglepaINStandardB UMETA(DisplayName ="pa-IN-Standard-B"),
    TTSVoiceGooglepaINStandardC UMETA(DisplayName ="pa-IN-Standard-C"),
    TTSVoiceGooglepaINWavenetA UMETA(DisplayName ="pa-IN-Wavenet-A"),
    TTSVoiceGooglepaINWavenetB UMETA(DisplayName ="pa-IN-Wavenet-B"),
    TTSVoiceGooglepaINWavenetC UMETA(DisplayName ="pa-IN-Wavenet-C"),
    TTSVoiceGoogleroROStandardA UMETA(DisplayName ="ro-RO-Standard-A"),
    TTSVoiceGoogleroROWavenetA UMETA(DisplayName ="ro-RO-Wavenet-A"),
    TTSVoiceGoogleruRUStandardA UMETA(DisplayName ="ru-RU-Standard-A"),
    TTSVoiceGoogleruRUStandardB UMETA(DisplayName ="ru-RU-Standard-B"),
    TTSVoiceGoogleruRUStandardC UMETA(DisplayName ="ru-RU-Standard-C"),
    TTSVoiceGoogleruRUWavenetA UMETA(DisplayName ="ru-RU-Wavenet-A"),
    TTSVoiceGoogleruRUWavenetB UMETA(DisplayName ="ru-RU-Wavenet-B"),
    TTSVoiceGoogleruRUWavenetC UMETA(DisplayName ="ru-RU-Wavenet-C"),
    TTSVoiceGooglesrrsStandardA UMETA(DisplayName ="sr-rs-Standard-A"),
    TTSVoiceGoogleskSKStandardA UMETA(DisplayName ="sk-SK-Standard-A"),
    TTSVoiceGoogleskSKWavenetA UMETA(DisplayName ="sk-SK-Wavenet-A"),
    TTSVoiceGoogleesESStandardA UMETA(DisplayName ="es-ES-Standard-A"),
    TTSVoiceGoogleesESStandardB UMETA(DisplayName ="es-ES-Standard-B"),
    TTSVoiceGoogleesESStandardC UMETA(DisplayName ="es-ES-Standard-C"),
    TTSVoiceGoogleesESWavenetB UMETA(DisplayName ="es-ES-Wavenet-B"),
    TTSVoiceGoogleesESWavenetC UMETA(DisplayName ="es-ES-Wavenet-C"),
    TTSVoiceGoogleesUSNeural2A UMETA(DisplayName ="es-US-Neural2-A"),
    TTSVoiceGoogleesUSNeural2C UMETA(DisplayName ="es-US-Neural2-C"),
    TTSVoiceGoogleesUSStandardA UMETA(DisplayName ="es-US-Standard-A"),
    TTSVoiceGoogleesUSStandardB UMETA(DisplayName ="es-US-Standard-B"),
    TTSVoiceGoogleesUSStandardC UMETA(DisplayName ="es-US-Standard-C"),
    TTSVoiceGoogleesUSWavenetA UMETA(DisplayName ="es-US-Wavenet-A"),
    TTSVoiceGoogleesUSWavenetB UMETA(DisplayName ="es-US-Wavenet-B"),
    TTSVoiceGoogleesUSWavenetC UMETA(DisplayName ="es-US-Wavenet-C"),
    TTSVoiceGooglesvSEStandardA UMETA(DisplayName ="sv-SE-Standard-A"),
    TTSVoiceGooglesvSEStandardB UMETA(DisplayName ="sv-SE-Standard-B"),
    TTSVoiceGooglesvSEStandardC UMETA(DisplayName ="sv-SE-Standard-C"),
    TTSVoiceGooglesvSEWavenetA UMETA(DisplayName ="sv-SE-Wavenet-A"),
    TTSVoiceGooglesvSEWavenetB UMETA(DisplayName ="sv-SE-Wavenet-B"),
    TTSVoiceGooglesvSEWavenetC UMETA(DisplayName ="sv-SE-Wavenet-C"),
    TTSVoiceGoogletaINStandardA UMETA(DisplayName ="ta-IN-Standard-A"),
    TTSVoiceGoogletaINStandardB UMETA(DisplayName ="ta-IN-Standard-B"),
    TTSVoiceGoogletaINWavenetA UMETA(DisplayName ="ta-IN-Wavenet-A"),
    TTSVoiceGoogletaINWavenetB UMETA(DisplayName ="ta-IN-Wavenet-B"),
    TTSVoiceGoogleteINStandardA UMETA(DisplayName ="te-IN-Standard-A"),
    TTSVoiceGoogleteINStandardB UMETA(DisplayName ="te-IN-Standard-B"),
    TTSVoiceGooglethTHStandardA UMETA(DisplayName ="th-TH-Standard-A"),
    TTSVoiceGoogletrTRStandardA UMETA(DisplayName ="tr-TR-Standard-A"),
    TTSVoiceGoogletrTRStandardB UMETA(DisplayName ="tr-TR-Standard-B"),
    TTSVoiceGoogletrTRStandardC UMETA(DisplayName ="tr-TR-Standard-C"),
    TTSVoiceGoogletrTRWavenetA UMETA(DisplayName ="tr-TR-Wavenet-A"),
    TTSVoiceGoogletrTRWavenetB UMETA(DisplayName ="tr-TR-Wavenet-B"),
    TTSVoiceGoogletrTRWavenetC UMETA(DisplayName ="tr-TR-Wavenet-C"),
    TTSVoiceGoogleukUAStandardA UMETA(DisplayName ="uk-UA-Standard-A"),
    TTSVoiceGoogleukUAWavenetA UMETA(DisplayName ="uk-UA-Wavenet-A"),
    TTSVoiceGoogleviVNStandardA UMETA(DisplayName ="vi-VN-Standard-A"),
    TTSVoiceGoogleviVNStandardB UMETA(DisplayName ="vi-VN-Standard-B"),
    TTSVoiceGoogleviVNStandardC UMETA(DisplayName ="vi-VN-Standard-C"),
    TTSVoiceGoogleviVNWavenetA UMETA(DisplayName ="vi-VN-Wavenet-A"),
    TTSVoiceGoogleviVNWavenetB UMETA(DisplayName ="vi-VN-Wavenet-B"),
    TTSVoiceGoogleviVNWavenetC UMETA(DisplayName ="vi-VN-Wavenet-C"),
};

UENUM(BlueprintType)
enum class ETTSVoiceAzure : uint8
{
    TTSVoiceAzureafZAAdriNeural UMETA(DisplayName = "af-ZA-AdriNeural"),
    TTSVoiceAzureafZAWillemNeural UMETA(DisplayName = "af-ZA-WillemNeural"),
    TTSVoiceAzuresqALAnilaNeural UMETA(DisplayName = "sq-AL-AnilaNeural"),
    TTSVoiceAzuresqALIlirNeural UMETA(DisplayName = "sq-AL-IlirNeural"),
    TTSVoiceAzureamETMekdesNeural UMETA(DisplayName = "am-ET-MekdesNeural"),
    TTSVoiceAzureamETAmehaNeural UMETA(DisplayName = "am-ET-AmehaNeural"),
    TTSVoiceAzurearDZAminaNeural UMETA(DisplayName = "ar-DZ-AminaNeural"),
    TTSVoiceAzurearDZIsmaelNeural UMETA(DisplayName = "ar-DZ-IsmaelNeural"),
    TTSVoiceAzurearBHLailaNeural UMETA(DisplayName = "ar-BH-LailaNeural"),
    TTSVoiceAzurearBHAliNeural UMETA(DisplayName = "ar-BH-AliNeural"),
    TTSVoiceAzurearEGSalmaNeural UMETA(DisplayName = "ar-EG-SalmaNeural"),
    TTSVoiceAzurearEGShakirNeural UMETA(DisplayName = "ar-EG-ShakirNeural"),
    TTSVoiceAzurearIQRanaNeural UMETA(DisplayName = "ar-IQ-RanaNeural"),
    TTSVoiceAzurearIQBasselNeural UMETA(DisplayName = "ar-IQ-BasselNeural"),
    TTSVoiceAzurearJOSanaNeural UMETA(DisplayName = "ar-JO-SanaNeural"),
    TTSVoiceAzurearJOTaimNeural UMETA(DisplayName = "ar-JO-TaimNeural"),
    TTSVoiceAzurearKWNouraNeural UMETA(DisplayName = "ar-KW-NouraNeural"),
    TTSVoiceAzurearKWFahedNeural UMETA(DisplayName = "ar-KW-FahedNeural"),
    TTSVoiceAzurearYESalehNeural UMETA(DisplayName = "ar-YE-SalehNeural"),
    TTSVoiceAzureazAZBabekNeural UMETA(DisplayName = "az-AZ-BabekNeural"),
    TTSVoiceAzureazAZBanuNeural UMETA(DisplayName = "az-AZ-BanuNeural"),
    TTSVoiceAzurebnBDNabanitaNeural UMETA(DisplayName = "bn-BD-NabanitaNeural"),
    TTSVoiceAzurebnBDPradeepNeural UMETA(DisplayName = "bn-BD-PradeepNeural"),
    TTSVoiceAzurebnINTanishaaNeural UMETA(DisplayName = "bn-IN-TanishaaNeural"),
    TTSVoiceAzurebnINBashkarNeural UMETA(DisplayName = "bn-IN-BashkarNeural"),
    TTSVoiceAzurebsBAVesnaNeural UMETA(DisplayName = "bs-BA-VesnaNeural"),
    TTSVoiceAzurebsBAGoranNeural UMETA(DisplayName = "bs-BA-GoranNeural"),
    TTSVoiceAzurebgBGKalinaNeural UMETA(DisplayName = "bg-BG-KalinaNeural"),
    TTSVoiceAzurebgBGBorislavNeural UMETA(DisplayName = "bg-BG-BorislavNeural"),
    TTSVoiceAzuremyMMNilarNeural UMETA(DisplayName = "my-MM-NilarNeural"),
    TTSVoiceAzuremyMMThihaNeural UMETA(DisplayName = "my-MM-ThihaNeural"),
    TTSVoiceAzurecaESAlbaNeural UMETA(DisplayName = "ca-ES-AlbaNeural"),
    TTSVoiceAzurecaESJoanaNeural UMETA(DisplayName = "ca-ES-JoanaNeural"),
    TTSVoiceAzurecaESEnricNeural UMETA(DisplayName = "ca-ES-EnricNeural"),
    TTSVoiceAzurezhHKHiuGaaiNeural UMETA(DisplayName = "zh-HK-HiuGaaiNeural"),
    TTSVoiceAzurezhHKHiuMaanNeural UMETA(DisplayName = "zh-HK-HiuMaanNeural"),
    TTSVoiceAzurezhHKWanLungNeural UMETA(DisplayName = "zh-HK-WanLungNeural"),
    TTSVoiceAzurezhCNXiaochenNeural UMETA(DisplayName = "zh-CN-XiaochenNeural"),
    TTSVoiceAzurezhCNXiaohanNeural UMETA(DisplayName = "zh-CN-XiaohanNeural"),
    TTSVoiceAzurezhCNXiaomoNeural UMETA(DisplayName = "zh-CN-XiaomoNeural"),
    TTSVoiceAzurezhCNXiaoqiuNeural UMETA(DisplayName = "zh-CN-XiaoqiuNeural"),
    TTSVoiceAzurezhCNXiaoruiNeural UMETA(DisplayName = "zh-CN-XiaoruiNeural"),
    TTSVoiceAzurezhCNXiaoshuangNeural UMETA(DisplayName = "zh-CN-XiaoshuangNeural"),
    TTSVoiceAzurezhCNXiaoxiaoNeural UMETA(DisplayName = "zh-CN-XiaoxiaoNeural"),
    TTSVoiceAzurezhCNXiaoxuanNeural UMETA(DisplayName = "zh-CN-XiaoxuanNeural"),
    TTSVoiceAzurehrHRGabrijelaNeural UMETA(DisplayName = "hr-HR-GabrijelaNeural"),
    TTSVoiceAzurehrHRSreckoNeural UMETA(DisplayName = "hr-HR-SreckoNeural"),
    TTSVoiceAzurecsCZVlastaNeural UMETA(DisplayName = "cs-CZ-VlastaNeural"),
    TTSVoiceAzurecsCZAntoninNeural UMETA(DisplayName = "cs-CZ-AntoninNeural"),
    TTSVoiceAzuredaDKChristelNeural UMETA(DisplayName = "da-DK-ChristelNeural"),
    TTSVoiceAzuredaDKJeppeNeural UMETA(DisplayName = "da-DK-JeppeNeural"),
    TTSVoiceAzurenlBEDenaNeural UMETA(DisplayName = "nl-BE-DenaNeural"),
    TTSVoiceAzurenlBEArnaudNeural UMETA(DisplayName = "nl-BE-ArnaudNeural"),
    TTSVoiceAzurenlNLColetteNeural UMETA(DisplayName = "nl-NL-ColetteNeural"),
    TTSVoiceAzurenlNLFennaNeural UMETA(DisplayName = "nl-NL-FennaNeural"),
    TTSVoiceAzurenlNLMaartenNeural UMETA(DisplayName = "nl-NL-MaartenNeural"),
    TTSVoiceAzureenAUNatashaNeural UMETA(DisplayName = "en-AU-NatashaNeural"),
    TTSVoiceAzureenAUWilliamNeural UMETA(DisplayName = "en-AU-WilliamNeural"),
    TTSVoiceAzureenCAClaraNeural UMETA(DisplayName = "en-CA-ClaraNeural"),
    TTSVoiceAzureenCALiamNeural UMETA(DisplayName = "en-CA-LiamNeural"),
    TTSVoiceAzureenHKYanNeural UMETA(DisplayName = "en-HK-YanNeural"),
    TTSVoiceAzureenHKSamNeural UMETA(DisplayName = "en-HK-SamNeural"),
    TTSVoiceAzureenINNeerjaNeural UMETA(DisplayName = "en-IN-NeerjaNeural"),
    TTSVoiceAzureenINPrabhatNeural UMETA(DisplayName = "en-IN-PrabhatNeural"),
    TTSVoiceAzureenIEEmilyNeural UMETA(DisplayName = "en-IE-EmilyNeural"),
    TTSVoiceAzureenIEConnorNeural UMETA(DisplayName = "en-IE-ConnorNeural"),
    TTSVoiceAzureenKEAsiliaNeural UMETA(DisplayName = "en-KE-AsiliaNeural"),
    TTSVoiceAzureenKEChilembaNeural UMETA(DisplayName = "en-KE-ChilembaNeural"),
    TTSVoiceAzureenNZMollyNeural UMETA(DisplayName = "en-NZ-MollyNeural"),
    TTSVoiceAzureenNZMitchellNeural UMETA(DisplayName = "en-NZ-MitchellNeural"),
    TTSVoiceAzureenNGEzinneNeural UMETA(DisplayName = "en-NG-EzinneNeural"),
    TTSVoiceAzureenNGAbeoNeural UMETA(DisplayName = "en-NG-AbeoNeural"),
    TTSVoiceAzureenPHRosaNeural UMETA(DisplayName = "en-PH-RosaNeural"),
    TTSVoiceAzureenPHJamesNeural UMETA(DisplayName = "en-PH-JamesNeural"),
    TTSVoiceAzureenSGLunaNeural UMETA(DisplayName = "en-SG-LunaNeural"),
    TTSVoiceAzureenSGWayneNeural UMETA(DisplayName = "en-SG-WayneNeural"),
    TTSVoiceAzureenZALeahNeural UMETA(DisplayName = "en-ZA-LeahNeural"),
    TTSVoiceAzureenZALukeNeural UMETA(DisplayName = "en-ZA-LukeNeural"),
    TTSVoiceAzureenTZImaniNeural UMETA(DisplayName = "en-TZ-ImaniNeural"),
    TTSVoiceAzureenTZElimuNeural UMETA(DisplayName = "en-TZ-ElimuNeural"),
    TTSVoiceAzureenGBAbbiNeural UMETA(DisplayName = "en-GB-AbbiNeural"),
    TTSVoiceAzureenGBBellaNeural UMETA(DisplayName = "en-GB-BellaNeural"),
    TTSVoiceAzureenGBHollieNeural UMETA(DisplayName = "en-GB-HollieNeural"),
    TTSVoiceAzureenGBLibbyNeural UMETA(DisplayName = "en-GB-LibbyNeural"),
    TTSVoiceAzureenGBMaisieNeural UMETA(DisplayName = "en-GB-MaisieNeural"),
    TTSVoiceAzureenGBOliviaNeural UMETA(DisplayName = "en-GB-OliviaNeural"),
    TTSVoiceAzureenGBSoniaNeural UMETA(DisplayName = "en-GB-SoniaNeural"),
    TTSVoiceAzureenUSAmberNeural UMETA(DisplayName = "en-US-AmberNeural"),
    TTSVoiceAzureenUSAriaNeural UMETA(DisplayName = "en-US-AriaNeural"),
    TTSVoiceAzureenUSAshleyNeural UMETA(DisplayName = "en-US-AshleyNeural"),
    TTSVoiceAzureenUSCoraNeural UMETA(DisplayName = "en-US-CoraNeural"),
    TTSVoiceAzureenUSElizabethNeural UMETA(DisplayName = "en-US-ElizabethNeural"),
    TTSVoiceAzureenUSJennyNeural UMETA(DisplayName = "en-US-JennyNeural"),
    TTSVoiceAzureenUSJennyMultilingualNeural UMETA(DisplayName = "en-US-JennyMultilingualNeural"),
    TTSVoiceAzureenUSMichelleNeural UMETA(DisplayName = "en-US-MichelleNeural"),
    TTSVoiceAzureenUSMonicaNeural UMETA(DisplayName = "en-US-MonicaNeural"),
    TTSVoiceAzureenUSSaraNeural UMETA(DisplayName = "en-US-SaraNeural"),
    TTSVoiceAzureenUSAnaNeural UMETA(DisplayName = "en-US-AnaNeural"),
    TTSVoiceAzureenUSBrandonNeural UMETA(DisplayName = "en-US-BrandonNeural"),
    TTSVoiceAzureenUSChristopherNeural UMETA(DisplayName = "en-US-ChristopherNeural"),
    TTSVoiceAzureenUSEricNeural UMETA(DisplayName = "en-US-EricNeural"),
    TTSVoiceAzureenUSGuyNeural UMETA(DisplayName = "en-US-GuyNeural"),
    TTSVoiceAzureenUSJacobNeural UMETA(DisplayName = "en-US-JacobNeural"),
    TTSVoiceAzureetEEAnuNeural UMETA(DisplayName = "et-EE-AnuNeural"),
    TTSVoiceAzureetEEKertNeural UMETA(DisplayName = "et-EE-KertNeural"),
    TTSVoiceAzurefilPHBlessicaNeural UMETA(DisplayName = "fil-PH-BlessicaNeural"),
    TTSVoiceAzurefilPHAngeloNeural UMETA(DisplayName = "fil-PH-AngeloNeural"),
    TTSVoiceAzurefiFINooraNeural UMETA(DisplayName = "fi-FI-NooraNeural"),
    TTSVoiceAzurefiFISelmaNeural UMETA(DisplayName = "fi-FI-SelmaNeural"),
    TTSVoiceAzurefiFIHarriNeural UMETA(DisplayName = "fi-FI-HarriNeural"),
    TTSVoiceAzurefrBECharlineNeural UMETA(DisplayName = "fr-BE-CharlineNeural"),
    TTSVoiceAzurefrBEGerardNeural UMETA(DisplayName = "fr-BE-GerardNeural"),
    TTSVoiceAzurefrCASylvieNeural UMETA(DisplayName = "fr-CA-SylvieNeural"),
    TTSVoiceAzurefrCAAntoineNeural UMETA(DisplayName = "fr-CA-AntoineNeural"),
    TTSVoiceAzurefrCAJeanNeural UMETA(DisplayName = "fr-CA-JeanNeural"),
    TTSVoiceAzurefrFRBrigitteNeural UMETA(DisplayName = "fr-FR-BrigitteNeural"),
    TTSVoiceAzurefrFRCelesteNeural UMETA(DisplayName = "fr-FR-CelesteNeural"),
    TTSVoiceAzurefrFRCoralieNeural UMETA(DisplayName = "fr-FR-CoralieNeural"),
    TTSVoiceAzurefrFRDeniseNeural UMETA(DisplayName = "fr-FR-DeniseNeural"),
    TTSVoiceAzureglESSabelaNeural UMETA(DisplayName = "gl-ES-SabelaNeural"),
    TTSVoiceAzureglESRoiNeural UMETA(DisplayName = "gl-ES-RoiNeural"),
    TTSVoiceAzurekaGEEkaNeural UMETA(DisplayName = "ka-GE-EkaNeural"),
    TTSVoiceAzurekaGEGiorgiNeural UMETA(DisplayName = "ka-GE-GiorgiNeural"),
    TTSVoiceAzuredeATIngridNeural UMETA(DisplayName = "de-AT-IngridNeural"),
    TTSVoiceAzuredeATJonasNeural UMETA(DisplayName = "de-AT-JonasNeural"),
    TTSVoiceAzuredeDEAmalaNeural UMETA(DisplayName = "de-DE-AmalaNeural"),
    TTSVoiceAzuredeDEElkeNeural UMETA(DisplayName = "de-DE-ElkeNeural"),
    TTSVoiceAzuredeDEGiselaNeural UMETA(DisplayName = "de-DE-GiselaNeural"),
    TTSVoiceAzuredeDEKatjaNeural UMETA(DisplayName = "de-DE-KatjaNeural"),
    TTSVoiceAzuredeCHLeniNeural UMETA(DisplayName = "de-CH-LeniNeural"),
    TTSVoiceAzuredeCHJanNeural UMETA(DisplayName = "de-CH-JanNeural"),
    TTSVoiceAzureelGRAthinaNeural UMETA(DisplayName = "el-GR-AthinaNeural"),
    TTSVoiceAzureelGRNestorasNeural UMETA(DisplayName = "el-GR-NestorasNeural"),
    TTSVoiceAzureguINDhwaniNeural UMETA(DisplayName = "gu-IN-DhwaniNeural"),
    TTSVoiceAzureguINNiranjanNeural UMETA(DisplayName = "gu-IN-NiranjanNeural"),
    TTSVoiceAzureheILHilaNeural UMETA(DisplayName = "he-IL-HilaNeural"),
    TTSVoiceAzureheILAvriNeural UMETA(DisplayName = "he-IL-AvriNeural"),
    TTSVoiceAzurehiINSwaraNeural UMETA(DisplayName = "hi-IN-SwaraNeural"),
    TTSVoiceAzurehiINMadhurNeural UMETA(DisplayName = "hi-IN-MadhurNeural"),
    TTSVoiceAzurehuHUNoemiNeural UMETA(DisplayName = "hu-HU-NoemiNeural"),
    TTSVoiceAzurehuHUTamasNeural UMETA(DisplayName = "hu-HU-TamasNeural"),
    TTSVoiceAzureisISGudrunNeural UMETA(DisplayName = "is-IS-GudrunNeural"),
    TTSVoiceAzureisISGunnarNeural UMETA(DisplayName = "is-IS-GunnarNeural"),
    TTSVoiceAzureidIDGadisNeural UMETA(DisplayName = "id-ID-GadisNeural"),
    TTSVoiceAzureidIDArdiNeural UMETA(DisplayName = "id-ID-ArdiNeural"),
    TTSVoiceAzuregaIEOrlaNeural UMETA(DisplayName = "ga-IE-OrlaNeural"),
    TTSVoiceAzuregaIEColmNeural UMETA(DisplayName = "ga-IE-ColmNeural"),
    TTSVoiceAzureitITElsaNeural UMETA(DisplayName = "it-IT-ElsaNeural"),
    TTSVoiceAzureitITIsabellaNeural UMETA(DisplayName = "it-IT-IsabellaNeural"),
    TTSVoiceAzureitITDiegoNeural UMETA(DisplayName = "it-IT-DiegoNeural"),
    TTSVoiceAzurejaJPNanamiNeural UMETA(DisplayName = "ja-JP-NanamiNeural"),
    TTSVoiceAzurejaJPKeitaNeural UMETA(DisplayName = "ja-JP-KeitaNeural"),
    TTSVoiceAzurejvIDSitiNeural UMETA(DisplayName = "jv-ID-SitiNeural"),
    TTSVoiceAzurejvIDDimasNeural UMETA(DisplayName = "jv-ID-DimasNeural"),
    TTSVoiceAzureknINSapnaNeural UMETA(DisplayName = "kn-IN-SapnaNeural"),
    TTSVoiceAzureknINGaganNeural UMETA(DisplayName = "kn-IN-GaganNeural"),
    TTSVoiceAzurekkKZAigulNeural UMETA(DisplayName = "kk-KZ-AigulNeural"),
    TTSVoiceAzurekkKZDauletNeural UMETA(DisplayName = "kk-KZ-DauletNeural"),
    TTSVoiceAzurekmKHSreymomNeural UMETA(DisplayName = "km-KH-SreymomNeural"),
    TTSVoiceAzurekmKHPisethNeural UMETA(DisplayName = "km-KH-PisethNeural"),
    TTSVoiceAzurekoKRSunHiNeural UMETA(DisplayName = "ko-KR-SunHiNeural"),
    TTSVoiceAzurekoKRInJoonNeural UMETA(DisplayName = "ko-KR-InJoonNeural"),
    TTSVoiceAzureloLAKeomanyNeural UMETA(DisplayName = "lo-LA-KeomanyNeural"),
    TTSVoiceAzureloLAChanthavongNeural UMETA(DisplayName = "lo-LA-ChanthavongNeural"),
    TTSVoiceAzurelvLVEveritaNeural UMETA(DisplayName = "lv-LV-EveritaNeural"),
    TTSVoiceAzurelvLVNilsNeural UMETA(DisplayName = "lv-LV-NilsNeural"),
    TTSVoiceAzureltLTOnaNeural UMETA(DisplayName = "lt-LT-OnaNeural"),
    TTSVoiceAzureltLTLeonasNeural UMETA(DisplayName = "lt-LT-LeonasNeural"),
    TTSVoiceAzuremkMKMarijaNeural UMETA(DisplayName = "mk-MK-MarijaNeural"),
    TTSVoiceAzuremkMKAleksandarNeural UMETA(DisplayName = "mk-MK-AleksandarNeural"),
    TTSVoiceAzuremsMYYasminNeural UMETA(DisplayName = "ms-MY-YasminNeural"),
    TTSVoiceAzuremsMYOsmanNeural UMETA(DisplayName = "ms-MY-OsmanNeural"),
    TTSVoiceAzuremlINSobhanaNeural UMETA(DisplayName = "ml-IN-SobhanaNeural"),
    TTSVoiceAzuremlINMidhunNeural UMETA(DisplayName = "ml-IN-MidhunNeural"),
    TTSVoiceAzuremtMTGraceNeural UMETA(DisplayName = "mt-MT-GraceNeural"),
    TTSVoiceAzuremtMTJosephNeural UMETA(DisplayName = "mt-MT-JosephNeural"),
    TTSVoiceAzuremrINAarohiNeural UMETA(DisplayName = "mr-IN-AarohiNeural"),
    TTSVoiceAzuremrINManoharNeural UMETA(DisplayName = "mr-IN-ManoharNeural"),
    TTSVoiceAzuremnMNYesuiNeural UMETA(DisplayName = "mn-MN-YesuiNeural"),
    TTSVoiceAzuremnMNBataaNeural UMETA(DisplayName = "mn-MN-BataaNeural"),
    TTSVoiceAzureneNPHemkalaNeural UMETA(DisplayName = "ne-NP-HemkalaNeural"),
    TTSVoiceAzureneNPSagarNeural UMETA(DisplayName = "ne-NP-SagarNeural"),
    TTSVoiceAzurenbNOIselinNeural UMETA(DisplayName = "nb-NO-IselinNeural"),
    TTSVoiceAzurenbNOPernilleNeural UMETA(DisplayName = "nb-NO-PernilleNeural"),
    TTSVoiceAzurenbNOFinnNeural UMETA(DisplayName = "nb-NO-FinnNeural"),
    TTSVoiceAzurepsAFLatifaNeural UMETA(DisplayName = "ps-AF-LatifaNeural"),
    TTSVoiceAzurepsAFGulNawazNeural UMETA(DisplayName = "ps-AF-GulNawazNeural"),
    TTSVoiceAzurefaIRDilaraNeural UMETA(DisplayName = "fa-IR-DilaraNeural"),
    TTSVoiceAzurefaIRFaridNeural UMETA(DisplayName = "fa-IR-FaridNeural"),
    TTSVoiceAzureplPLAgnieszkaNeural UMETA(DisplayName = "pl-PL-AgnieszkaNeural"),
    TTSVoiceAzureplPLZofiaNeural UMETA(DisplayName = "pl-PL-ZofiaNeural"),
    TTSVoiceAzureplPLMarekNeural UMETA(DisplayName = "pl-PL-MarekNeural"),
    TTSVoiceAzureptBRFranciscaNeural UMETA(DisplayName = "pt-BR-FranciscaNeural"),
    TTSVoiceAzureptBRAntonioNeural UMETA(DisplayName = "pt-BR-AntonioNeural"),
    TTSVoiceAzureptPTFernandaNeural UMETA(DisplayName = "pt-PT-FernandaNeural"),
    TTSVoiceAzureroROAlinaNeural UMETA(DisplayName = "ro-RO-AlinaNeural"),
    TTSVoiceAzureroROEmilNeural UMETA(DisplayName = "ro-RO-EmilNeural"),
    TTSVoiceAzureruRUDariyaNeural UMETA(DisplayName = "ru-RU-DariyaNeural"),
    TTSVoiceAzureruRUSvetlanaNeural UMETA(DisplayName = "ru-RU-SvetlanaNeural"),
    TTSVoiceAzureruRUDmitryNeural UMETA(DisplayName = "ru-RU-DmitryNeural"),
    TTSVoiceAzuresrRSSophieNeural UMETA(DisplayName = "sr-RS-SophieNeural"),
    TTSVoiceAzuresrRSNicholasNeural UMETA(DisplayName = "sr-RS-NicholasNeural"),
    TTSVoiceAzuresiLKThiliniNeural UMETA(DisplayName = "si-LK-ThiliniNeural"),
    TTSVoiceAzuresiLKSameeraNeural UMETA(DisplayName = "si-LK-SameeraNeural"),
    TTSVoiceAzureskSKViktoriaNeural UMETA(DisplayName = "sk-SK-ViktoriaNeural"),
    TTSVoiceAzureskSKLukasNeural UMETA(DisplayName = "sk-SK-LukasNeural"),
    TTSVoiceAzureslSIPetraNeural UMETA(DisplayName = "sl-SI-PetraNeural"),
    TTSVoiceAzureslSIRokNeural UMETA(DisplayName = "sl-SI-RokNeural"),
    TTSVoiceAzuresoSOUbaxNeural UMETA(DisplayName = "so-SO-UbaxNeural"),
    TTSVoiceAzuresoSOMuuseNeural UMETA(DisplayName = "so-SO-MuuseNeural"),
    TTSVoiceAzureesESElviraNeural UMETA(DisplayName = "es-ES-ElviraNeural"),
    TTSVoiceAzureesESAlvaroNeural UMETA(DisplayName = "es-ES-AlvaroNeural"),
    TTSVoiceAzureesUYValentinaNeural UMETA(DisplayName = "es-UY-ValentinaNeural"),
    TTSVoiceAzureesUYMateoNeural UMETA(DisplayName = "es-UY-MateoNeural"),
    TTSVoiceAzureesUSPalomaNeural UMETA(DisplayName = "es-US-PalomaNeural"),
    TTSVoiceAzureesUSAlonsoNeural UMETA(DisplayName = "es-US-AlonsoNeural"),
    TTSVoiceAzureesVEPaolaNeural UMETA(DisplayName = "es-VE-PaolaNeural"),
    TTSVoiceAzureesVESebastianNeural UMETA(DisplayName = "es-VE-SebastianNeural"),
    TTSVoiceAzuresuIDTutiNeural UMETA(DisplayName = "su-ID-TutiNeural"),
    TTSVoiceAzuresuIDJajangNeural UMETA(DisplayName = "su-ID-JajangNeural"),
    TTSVoiceAzureswKEZuriNeural UMETA(DisplayName = "sw-KE-ZuriNeural"),
    TTSVoiceAzureswKERafikiNeural UMETA(DisplayName = "sw-KE-RafikiNeural"),
    TTSVoiceAzureswTZRehemaNeural UMETA(DisplayName = "sw-TZ-RehemaNeural"),
    TTSVoiceAzureswTZDaudiNeural UMETA(DisplayName = "sw-TZ-DaudiNeural"),
    TTSVoiceAzuresvSEHilleviNeural UMETA(DisplayName = "sv-SE-HilleviNeural"),
    TTSVoiceAzuresvSESofieNeural UMETA(DisplayName = "sv-SE-SofieNeural"),
    TTSVoiceAzuresvSEMattiasNeural UMETA(DisplayName = "sv-SE-MattiasNeural"),
    TTSVoiceAzuretaINPallaviNeural UMETA(DisplayName = "ta-IN-PallaviNeural"),
    TTSVoiceAzuretaINValluvarNeural UMETA(DisplayName = "ta-IN-ValluvarNeural"),
    TTSVoiceAzuretaMYKaniNeural UMETA(DisplayName = "ta-MY-KaniNeural"),
    TTSVoiceAzuretaMYSuryaNeural UMETA(DisplayName = "ta-MY-SuryaNeural"),
    TTSVoiceAzureteINShrutiNeural UMETA(DisplayName = "te-IN-ShrutiNeural"),
    TTSVoiceAzureteINMohanNeural UMETA(DisplayName = "te-IN-MohanNeural"),
    TTSVoiceAzurethTHAcharaNeural UMETA(DisplayName = "th-TH-AcharaNeural"),
    TTSVoiceAzurethTHPremwadeeNeural UMETA(DisplayName = "th-TH-PremwadeeNeural"),
    TTSVoiceAzurethTHNiwatNeural UMETA(DisplayName = "th-TH-NiwatNeural"),
    TTSVoiceAzuretrTREmelNeural UMETA(DisplayName = "tr-TR-EmelNeural"),
    TTSVoiceAzuretrTRAhmetNeural UMETA(DisplayName = "tr-TR-AhmetNeural"),
    TTSVoiceAzureukUAPolinaNeural UMETA(DisplayName = "uk-UA-PolinaNeural"),
    TTSVoiceAzureukUAOstapNeural UMETA(DisplayName = "uk-UA-OstapNeural"),
    TTSVoiceAzureurINGulNeural UMETA(DisplayName = "ur-IN-GulNeural"),
    TTSVoiceAzureurINSalmanNeural UMETA(DisplayName = "ur-IN-SalmanNeural"),
    TTSVoiceAzureurPKUzmaNeural UMETA(DisplayName = "ur-PK-UzmaNeural"),
    TTSVoiceAzureurPKAsadNeural UMETA(DisplayName = "ur-PK-AsadNeural"),
    TTSVoiceAzureuzUZMadinaNeural UMETA(DisplayName = "uz-UZ-MadinaNeural"),
    TTSVoiceAzureuzUZSardorNeural UMETA(DisplayName = "uz-UZ-SardorNeural"),
    TTSVoiceAzureviVNHoaiMyNeural UMETA(DisplayName = "vi-VN-HoaiMyNeural"),
    TTSVoiceAzureviVNNamMinhNeural UMETA(DisplayName = "vi-VN-NamMinhNeural"),
    TTSVoiceAzurecyGBNiaNeural UMETA(DisplayName = "cy-GB-NiaNeural"),
    TTSVoiceAzurecyGBAledNeural UMETA(DisplayName = "cy-GB-AledNeural"),
    TTSVoiceAzurezuZAThandoNeural UMETA(DisplayName = "zu-ZA-ThandoNeural"),
    TTSVoiceAzurezuZAThembaNeural UMETA(DisplayName = "zu-ZA-ThembaNeural"),
};

USTRUCT(BlueprintType)
struct METAHUMANSDK_API FATLMappingsInfo
{
    GENERATED_BODY()

public:
    FATLMappingsInfo() {}
    FATLMappingsInfo(UMetahumanSDKMappingsAsset* InMappingsAsset, UMetahumanSDKBoneMappingAsset* InBoneMappingAsset, bool bInSetUpForMetahumans) : MappingsAsset(InMappingsAsset), BoneMappingAsset(InBoneMappingAsset), bSetUpForMetahumans(bInSetUpForMetahumans) {}
    FATLMappingsInfo(UPoseAsset* InPoseAsset, UMetahumanSDKBoneMappingAsset* InBoneMappingAsset, bool bInSetUpForMetahumans) : PoseAsset(InPoseAsset), BoneMappingAsset(InBoneMappingAsset), bSetUpForMetahumans(bInSetUpForMetahumans) {}

    bool Validate() const
    {
        return !(IsValid(MappingsAsset) && IsValid(PoseAsset));
    }

    UPoseAsset* GetPoseAsset() const
    {
        return PoseAsset;
    }

    UMetahumanSDKMappingsAsset* GetMappingsAsset() const
    {
        return MappingsAsset;
    }

    UMetahumanSDKBoneMappingAsset* GetBoneMappingAsset() const
    {
        return BoneMappingAsset;
    }

    bool ShouldSetUpForMetahumans() const
    {
        return bSetUpForMetahumans;
    }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLMappingsInfo")
    UPoseAsset* PoseAsset = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLMappingsInfo")
    UMetahumanSDKMappingsAsset* MappingsAsset = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLMappingsInfo")
    UMetahumanSDKBoneMappingAsset* BoneMappingAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLMappingsInfo")
    bool bSetUpForMetahumans = false;
};

USTRUCT(BlueprintType)
struct METAHUMANSDK_API FMetahumanSDKATLInput
{
    GENERATED_BODY()

    bool Validate(FString& Error) const
    {
        if (!IsValid(Sound))
        {
            Error = "Provide valid Sound for ATL request!";
            return false;
        }

        if (!IsValid(Skeleton))
        {
            Error = "Provide valid Skeleton for ATL request!";
            return false;
        }

        if (!MappingsInfo.Validate())
        {
            Error = "You should not provide both mappings and pose asset!";
            return false;
        }
        
        return true;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    USkeleton* Skeleton = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    TEnumAsByte<ERichCurveInterpMode> BlendShapeCurveInterpMode = RCIM_Linear;

    /* Blendshapes and bones mappings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    FATLMappingsInfo MappingsInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    USoundWave* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    FString RigVersion = "shapes_2.0";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    FString Engine = TEXT("latest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    EExplicitEmotion ExplicitEmotion = EExplicitEmotion::ECalm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    bool EyeMovementModeMocap = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    bool NeckMovementModeMocap = false;
};

USTRUCT(BlueprintType)
struct METAHUMANSDK_API FMetahumanSDKTTSInput
{
    GENERATED_BODY()

    bool Validate(FString& Error) const
    {
        if (Text.Len() == 0)
        {
            Error = "Provide text for TTS request!";
        }

        return Text.Len() > 0;
    }
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKTTSInput")
    FString Text = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKTTSInput")
    FString TTSEngine = TEXT("latest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKTTSInput")
    FString VoiceID = TEXT("default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKTTSInput")
    FString TextType = TEXT("application/text");
};

USTRUCT(BlueprintType)
struct METAHUMANSDK_API FMetahumanSDKChatInput
{
    GENERATED_BODY()

    bool Validate(FString& Error) const
    {
        if (Text.Len() == 0)
        {
            Error = "Provide text for TTS request!";
        }

        return Text.Len() > 0;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKChatInput")
    FString Text = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKATLInput")
    EChatLanguage Language = EChatLanguage::CLenUS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKChatInput")
    FString Engine = TEXT("latest");
};

UENUM(BlueprintType)
enum class EComboMode : uint8
{
    ECHAT_TTS_ATL,
    ETTS_ATL,
    ECHAT_TTS
};

USTRUCT(BlueprintType)
struct METAHUMANSDK_API FMetahumanSDKComboInput
{
    GENERATED_BODY()

    bool Validate(FString& Error) const
    {
        if (ComboMode == EComboMode::ECHAT_TTS_ATL)
        {
            return ChatInput.Validate(Error);
        }
        else if (ComboMode == EComboMode::ECHAT_TTS)
        {
            return ChatInput.Validate(Error);
        }
        else if (ComboMode == EComboMode::ETTS_ATL)
        {
            return TTSInput.Validate(Error);
        }

        return true;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKComboInput")
    EComboMode ComboMode = EComboMode::ECHAT_TTS_ATL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKComboInput")
    FMetahumanSDKTTSInput TTSInput;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKComboInput")
    FMetahumanSDKATLInput ATLInput;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetahumanSDKComboInput")
    FMetahumanSDKChatInput ChatInput;
};