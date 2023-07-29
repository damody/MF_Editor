/*
*  Copyright (c) 2020-2020 Damody(t1238142000@gmail.com).
*  All rights reserved.
*  @ Date : 2020/08/06
*
*/
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBPMqttModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void* ExampleLibraryHandle;
};
