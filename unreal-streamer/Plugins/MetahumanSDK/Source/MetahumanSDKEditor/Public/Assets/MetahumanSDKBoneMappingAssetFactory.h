#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "MetahumanSDKBoneMappingAssetFactory.generated.h"

UCLASS()
class UMetahumanSDKBoneMappingAssetFactory : public UFactory
{
	GENERATED_BODY()

	UMetahumanSDKBoneMappingAssetFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
