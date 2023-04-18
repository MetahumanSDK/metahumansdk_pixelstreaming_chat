using UnrealBuildTool;
using System.IO;

public class MetahumanSDK : ModuleRules
{
    public MetahumanSDK(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;
        bEnforceIWYU = true;

        PublicIncludePaths.Add(ModuleDirectory + "/Public/Actions");
        PublicIncludePaths.Add(ModuleDirectory + "/Public/AnimGraphNodes");
        PublicIncludePaths.Add(ModuleDirectory + "/Public/AssetFactories");
        PublicIncludePaths.Add(ModuleDirectory + "/Public/Assets");
        PrivateIncludePaths.Add(ModuleDirectory + "/Private/Actions");
        PrivateIncludePaths.Add(ModuleDirectory + "/Private/AnimGraphNodes");
        PrivateIncludePaths.Add(ModuleDirectory + "/Private/AssetFactories");
        PrivateIncludePaths.Add(ModuleDirectory + "/Private/Assets");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Json",
                // ... add other public dependencies that you statically link with here ...
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "HTTP",
                "AudioPlatformConfiguration", // UE5
                "AudioMixer",
                "Projects", // IPluginManager
                "kubazip"
                // ... add private dependencies that you statically link with here ...    
            }
        );

        if (Target.Version.MinorVersion > 25)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "AudioPlatformConfiguration"

                    // ... add private dependencies that you statically link with here ...    
                }
            );
        }
        // add support for ogg vorbis files for runtime sound wave construction
        AddEngineThirdPartyPrivateStaticDependencies(Target,
            "UEOgg",
            "Vorbis",
            "VorbisFile",
            "libOpus",
            "UELibSampleRate"
        );

        PublicDefinitions.AddRange(
            new string[]
            {
                "DR_WAV_IMPLEMENTATION=1"
            }
        );
        
        // This is used to get FSoundQualityInfo struct for IAudioEncoder.
        PrivateIncludePathModuleNames.Add("TargetPlatform");

        PrivateIncludePathModuleNames.Add("BinkAudioDecoder");
        
        
        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "AssetTools",
                    "AssetRegistry"
                }
            );
        }        
    }
}
