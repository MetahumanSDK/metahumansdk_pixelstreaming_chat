#pragma once

#include "CoreMinimal.h"
#include "MetahumanSDKResponses.h"
#include "Curves/RichCurve.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "MetahumanSDKAPIManager.h"
#include "MetahumanSDKRequests.h"
#include "MetahumanSDKRequestManager.h"
#include "Internationalization/Culture.h"
#include "MetahumanSDKAnimSequenceAssetFactory.generated.h"

DECLARE_DELEGATE_OneParam(FOnATLRequestCompleted, TSharedPtr<FGenericResponse>);

/**
 * Implements a factory for UAnimSequence objects.
 */
UCLASS()
class METAHUMANSDKEDITOR_API UMetahumanSDKAnimSequenceAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMetahumanSDKAnimSequenceAssetFactory(const FObjectInitializer& ObjectInitializer);

	// Import file as text
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	
	/* New assets that don't override this function are automatically placed into the "Miscellaneous" category in the editor */
	virtual FText GetDisplayName() const override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual bool FactoryCanImport(const FString& Filename) override;

	TWeakObjectPtr<USoundWave> SourceSoundWave;
	
	static int PrevSize;
	static double PrevTime;
	static double PrevAudioLength;

protected:
	void MakeATLRequest(const FATLRequest& Request, FOnATLRequestCompleted CompletedDelegate = nullptr);
	FString GenerateRequestJson(const FGenericRequest& Request) const;
	
private:
	// UMetahumanSDKAPIManager* APIManager;
	// TSharedPtr<class FMetahumanSDKRequestManager> RequestsManager;
	TSharedPtr<FGenericResponse> LastATLResponse;

	bool GenerateAPIToken(FString& OutAPIToken);
};