
using System.IO;
using UnrealBuildTool;
// Copyright 2017 Tracer Interactive, LLC. All Rights Reserved.
public class WebUI : ModuleRules
{
	public WebUI(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseSharedPCHs;
        SharedPCHHeaderFile = "Private/WebUIPrivatePCH.h";

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
                "Slate",
                "SlateCore",
				"UMG",
                "HeadMountedDisplay",
                "ImageWrapper"
            }
		);

            
		PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Json",
            "InputCore"
        });
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            if (Target.Type != TargetType.Server)
            {
                PublicDependencyModuleNames.AddRange(new string[]
                {
                    "WebBrowserExtension"
                });
            }
        }
	}
}

