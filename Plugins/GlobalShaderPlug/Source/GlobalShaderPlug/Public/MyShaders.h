#pragma once

#include "GlobalShader.h"
#include "Shader.h"
#include <RHIResources.h>

BEGIN_UNIFORM_BUFFER_STRUCT(FMyUniform, )
SHADER_PARAMETER(FLinearColor, Color1)
SHADER_PARAMETER(FVector4, Color2)
SHADER_PARAMETER(float, LerpValue)
END_UNIFORM_BUFFER_STRUCT()
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FMyUniform, "MyUniform");



class FMyShaderBase : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FMyShaderBase, NonVirtual);
public:
	FMyShaderBase(){}
	FMyShaderBase(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		MainColorVal.Bind(Initializer.ParameterMap, TEXT("MainColor"));
		MainTextureVal.Bind(Initializer.ParameterMap, TEXT("MainTexture"));
		MainTextureSamplerVal.Bind(Initializer.ParameterMap, TEXT("MainTextureSampler"));
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

	void SetParameters(FRHICommandListImmediate& RHICmdList, const FLinearColor& MyColor, FRHITexture2D* InTexture)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), MainColorVal, MyColor);
		SetTextureParameter(
			RHICmdList, 
			RHICmdList.GetBoundPixelShader(), 
			MainTextureVal, 
			MainTextureSamplerVal, 
			TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), 
			InTexture);

		FMyUniform uni;
		uni.Color1 = FLinearColor::Blue;
		uni.Color2 = FLinearColor::Green;
		uni.LerpValue = 0.5f;

		TUniformBufferRef<FMyUniform> Data = TUniformBufferRef<FMyUniform>::CreateUniformBufferImmediate(uni, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), GetUniformBufferParameter<FMyUniform>(), Data);
	}

private:
	LAYOUT_FIELD(FShaderParameter, MainColorVal);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureVal);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerVal);
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