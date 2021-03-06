// Copyright Epic Games, Inc. All Rights Reserved.

#include "GlobalShaderPlug.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include <ShaderCore.h>

#define LOCTEXT_NAMESPACE "FGlobalShaderPlugModule"

void FGlobalShaderPlugModule::StartupModule()
{
	FString ShaderPath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("GlobalShaderPlug"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/GlobalShaderPlug"), ShaderPath);
}

void FGlobalShaderPlugModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGlobalShaderPlugModule, GlobalShaderPlug)