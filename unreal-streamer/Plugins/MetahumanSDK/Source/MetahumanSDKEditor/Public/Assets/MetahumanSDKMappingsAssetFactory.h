#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "MetahumanSDKMappingsAssetFactory.generated.h"

UCLASS()
class UMetahumanSDKMappingsAssetFactory : public UFactory
{
	GENERATED_BODY()

	UMetahumanSDKMappingsAssetFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
