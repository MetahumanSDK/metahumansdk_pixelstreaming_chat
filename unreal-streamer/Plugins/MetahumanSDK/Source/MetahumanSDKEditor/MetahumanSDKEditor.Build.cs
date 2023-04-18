using UnrealBuildTool;

public class MetahumanSDKEditor : ModuleRules
{
	public MetahumanSDKEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseUnity = false;
		bEnforceIWYU = true;

        PrivateIncludePaths.AddRange(
			new string[] {
				"MetahumanSDKEditor/Private",
				
				// ... add other private include paths required here ...
			});

		PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "Settings",
                "AssetTools",
                "MetahumanSDK"
            });

		PrivateIncludePaths.Add(ModuleDirectory + "/Private/AnimGraphNodes");
		PrivateIncludePaths.Add(ModuleDirectory + "/Private/Assets");
		PublicIncludePaths.Add(ModuleDirectory + "/Public/AnimGraphNodes");
		PublicIncludePaths.Add(ModuleDirectory + "/Public/Assets");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"MetahumanSDK",
				"AssetTools",
				"CoreUObject",
				"Engine",
				"InputCore",
				"HTTP",
				"kubazip",
				"ToolMenus",
				"ContentBrowser",
				"ContentBrowserData",
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
                "AssetTools",
                "UnrealEd",     // for FAssetEditorManager
                "KismetWidgets",
                "KismetCompiler",
                "BlueprintGraph",
                "GraphEditor",
                "Kismet",       // for FWorkflowCentricApplication
                "PropertyEditor",
                "EditorStyle",
                "Sequencer",
                "DetailCustomizations",
                "Settings",
				"UnrealEd",
				"AssetRegistry",
				"MainFrame",
				"AssetTools",
				"ContentBrowser",
				"MainFrame",
				"AnimGraph",
				"ToolMenus",
				"Json",
				"MetahumanSDK",
				"ContentBrowser",
				"ContentBrowserData",
			});

    }
}
