// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
using System.IO;
using UnrealBuildTool;

public class zlibLibrary : ModuleRules
{
	public zlibLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
	        // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Win64", "zlibwapi.lib"));

            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "lib", "Win64", "zlibwapi.dll"));

            // Ensure that the DLL is staged along with the executable
            RuntimeDependencies.Add("zlibwapi.dll");
            // copy dll
            // RuntimeDependencies.Add("$(TargetOutputDir)/paho-mqtt3as.dll", Path.Combine(PluginDirectory, "Source/ThirdParty/mqtt/lib/Win64/paho-mqtt3as.dll"));
        }
	}
}
