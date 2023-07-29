// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
using System.IO;
using UnrealBuildTool;

public class mqtt : ModuleRules
{
	public mqtt(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
			PublicSystemLibraries.Add("ws2_32.lib");

            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Win64", "paho-mqtt3as-static.lib"));

            // Delay-load the DLL, so we can load it from the right place first
            //PublicDelayLoadDLLs.Add("paho-mqtt3as.dll");

            // Ensure that the DLL is staged along with the executable
            //RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/mqtt/lib/Win64/paho-mqtt3as.dll");
            // copy dll
            //RuntimeDependencies.Add("$(TargetOutputDir)/paho-mqtt3as.dll", Path.Combine(PluginDirectory, "Source/ThirdParty/mqtt/lib/Win64/paho-mqtt3as.dll"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
			PublicSystemLibraryPaths.Add(Path.Combine(ModuleDirectory, "Linux"));
			PublicSystemLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Linux", "libpaho-mqtt3as.a"));
        }
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Android", "ARMv7", "libpaho-mqtt3as.a"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Android", "ARM64", "libpaho-mqtt3as.a"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Android", "x86", "libpaho-mqtt3as.a"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Android", "x64", "libpaho-mqtt3as.a"));
		}
		else
        {
            string Err = string.Format("{0} is made to depend on {1}. We want to avoid this, please correct module dependencies.", Target.Platform.ToString(), this.ToString());
            System.Console.WriteLine(Err);
            throw new BuildException(Err);
        }
	}
}
