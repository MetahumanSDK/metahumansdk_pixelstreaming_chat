#pragma once

#include "CoreMinimal.h"
#include "MetahumanSDKRequests.h"
#include "MetahumanSDKResponses.h"
#include "UObject/Object.h"
#include "Internationalization/Culture.h"
#include "MetahumanSDKSoundWaveAssetFactory.generated.h"

DECLARE_DELEGATE_OneParam(FOnTTSRequestCompleted, TSharedPtr<FGenericResponse>);

/**
 * 
 */
UCLASS()
class METAHUMANSDKEDITOR_API UMetahumanSDKSoundWaveAssetFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UMetahumanSDKSoundWaveAssetFactory(const FObjectInitializer& ObjectInitializer);

	// Import file as text
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	
	/* New assets that don't override this function are automatically placed into the "Miscellaneous" category in the editor */
	virtual FText GetDisplayName() const override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	
	static int PrevSize;
	static double PrevTime;

protected:
	void MakeTTSRequest(const FTTSRequest& Request, FOnTTSRequestCompleted CompletedDelegate = nullptr);
	FString GenerateRequestJson(const FGenericRequest& Request) const;
	
private:
	TSharedPtr<FGenericResponse> LastTTSResponse;

	bool GenerateAPIToken(FString& OutAPIToken);
};
