#pragma once

#include "GlobalShader.h"
#include "Shader.h"

class FMyShaderBase : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FMyShaderBase, NonVirtual);
public:
	FMyShaderBase(){}
	FMyShaderBase(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	void SetParameters(FRHICommandListImmediate& RHICmdList, const FLinearColor& MyColor)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), MainColorVal, MyColor);
	}

private:
	LAYOUT_FIELD(FShaderParameter, MainColorVal);
};

class FShader_VS: public FMyShaderBase
{
	DECLARE_GLOBAL_SHADER(FShader_VS);
public:
	FShader_VS() {}
	FShader_VS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:FMyShaderBase(Initializer)
	{ }	
};

class FShader_PS : public FMyShaderBase
{
	DECLARE_GLOBAL_SHADER(FShader_PS);
public:
	FShader_PS() {}
	FShader_PS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:FMyShaderBase(Initializer)
	{
	}
};

IMPLEMENT_SHADER_TYPE(, FShader_VS, TEXT("/GlobalShaderPlug/MyGlobalShader.usf"),TEXT("MainVS"),SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FShader_PS, TEXT("/GlobalShaderPlug/MyGlobalShader.usf"), TEXT("MainPS"), SF_Pixel)