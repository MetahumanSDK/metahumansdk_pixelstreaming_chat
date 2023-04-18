#include "MetahumanSDKSettings.h"

#include "Interfaces/IPluginManager.h"

UMetahumanSDKSettings::UMetahumanSDKSettings()
{
	LoadConfig();
}

const FString UMetahumanSDKSettings::GetPluginVersion()
{
	if (PluginVersion.IsEmpty())
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("MetahumanSDK"));
	
		if (Plugin.IsValid())
		{
			PluginVersion = Plugin->GetDescriptor().VersionName;
		}		
	}

	return PluginVersion;
}
