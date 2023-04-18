using UnrealBuildTool;
using System;
using System.IO;

public class kubazip : ModuleRules
{
	public kubazip(ReadOnlyTargetRules Target): base(Target)
	{
        Type = ModuleType.External;
		
		// link kubazip
        string IncludeDirectory = Path.Combine(ModuleDirectory, "distribution", "include");
        string LibDirectory = Path.Combine(ModuleDirectory, "distribution", "lib");

        PublicIncludePaths.Add(IncludeDirectory);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, "kubazip.lib"));
        }
		else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
			PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, "kubazip.a"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
			PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, "kubazip.mac.a"));
		}
    }
}
