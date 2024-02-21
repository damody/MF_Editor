
using System.IO;
using UnrealBuildTool;
// Copyright 2017 Tracer Interactive, LLC. All Rights Reserved.
public class WebUI : ModuleRules
{
	public WebUI(ReadOnlyTargetRules Target) : base(Target)
	{
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
	}
}

