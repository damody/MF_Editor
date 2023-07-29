/*
*  Copyright (c) 2020-2020 Damody(t1238142000@gmail.com).
*  All rights reserved.
*  @ Date : 2021/04/10
*
*/
#include "BPMqtt.h"
// #include "Modules/ModuleManager.h"
// #include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FBPMqttModule"

void FBPMqttModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// Get the base directory of this plugin
// 	FString BaseDir = IPluginManager::Get().FindPlugin("mqtt")->GetBaseDir();
// 
// 	// Add on the relative location of the third party dll and load it
// 	FString LibraryPath;
// #if PLATFORM_WINDOWS
// 	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/mqtt/lib/Win64/Release/paho-mqtt3a.dll"));
// #elif PLATFORM_LINUX
// 	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TheMqttLibrary/Mac/Release/libExampleLibrary.dylib"));
// #endif // PLATFORM_WINDOWS
// 
// 	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

}

void FBPMqttModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
// 	FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
// 	ExampleLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBPMqttModule, BPMqtt)