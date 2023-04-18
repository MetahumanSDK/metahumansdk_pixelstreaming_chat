#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMetahumanSDK, Log, All);

class FMetahumanSDKEditorModule : public IModuleInterface
{

public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
    void RegisterSettings();
    void UnregisterSettings();
	void RegisterMenus();

private:
	TArray< TSharedPtr<class IAssetTypeActions> > CurrentActions;
};
