#include "UtilityFunctions.h"
#include "MyShaders.h"
#include <Engine/TextureRenderTarget2D.h>



struct FVertex_Pos_UV
{
	FVector4 Position;
	FVector2D UV;
};

class FVertex_Pos_UV_Declaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FVertex_Pos_UV);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FVertex_Pos_UV, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FVertex_Pos_UV, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI->Release();
	}
};

static void DrawToQuad_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	FLinearColor MyColor,
	FRHITexture2D* MyRHITexture
	)
{
	check(IsInRenderingThread());

	FRHITexture2D* RenderTargetTexture = OutputRenderTargetResource->GetRenderTargetTexture();
	RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::SRVMask, ERHIAccess::RTV));
	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawColorPass"));
	{
		// SetViewport
		RHICmdList.SetViewport(
			0, 0, 0.f,
			OutputRenderTargetResource->GetSizeX(), OutputRenderTargetResource->GetSizeY(), 1.f);

		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
		TShaderMapRef<FShader_VS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FShader_PS> PixelShader(GlobalShaderMap);

		// Set the graphic pipeline state.  
		FVertex_Pos_UV_Declaration VertexDesc;
		VertexDesc.InitRHI();

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDesc.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		PixelShader->SetParameters(RHICmdList, MyColor, MyRHITexture);

		// Vertex Buffer Begins --------------------------
		FRHIResourceCreateInfo createInfo;
		FVertexBufferRHIRef MyVertexBufferRHI = RHICreateVertexBuffer(sizeof(FVertex_Pos_UV) * 4, BUF_Static, createInfo);
		void* VoidPtr = RHILockVertexBuffer(MyVertexBufferRHI, 0, sizeof(FVertex_Pos_UV) * 4, RLM_WriteOnly);

		FVertex_Pos_UV v[4];
		// LT
		v[0].Position = FVector4(-1.0f, 1.0f, 0.0f, 1.0f);
		v[0].UV = FVector2D(0, 1.0f);
		// RT
		v[1].Position = FVector4(1.0f, 1.0f, 0.0f, 1.0f);
		v[1].UV = FVector2D(1.0f, 1.0f);
		// LB
		v[2].Position = FVector4(-1.0f, -1.0f, 0.0f, 1.0f);
		v[2].UV = FVector2D(0.0f, 0.0f);
		// RB
		v[3].Position = FVector4(1.0f, -1.0f, 0.0f, 1.0f);
		v[3].UV = FVector2D(1.0f, 0.0f);

		FMemory::Memcpy(VoidPtr, &v, sizeof(FVertex_Pos_UV) * 4);
		RHIUnlockVertexBuffer(MyVertexBufferRHI);
		// Vertex Buffer Ends --------------------------

		// Index Buffer Begins--------------------
		static const uint16 Indices[6] = 
		{	0,1,2,
			2,1,3	};

		FRHIResourceCreateInfo IndexBufferCreateInfo;
		FIndexBufferRHIRef MyIndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * 6, BUF_Static, IndexBufferCreateInfo);
		void* VoidPtr2 = RHILockIndexBuffer(MyIndexBufferRHI, 0, sizeof(uint16) * 6, RLM_WriteOnly);
		FMemory::Memcpy(VoidPtr2, Indices, sizeof(uint16) * 6);
		
		RHIUnlockIndexBuffer(MyIndexBufferRHI);
		// Index Buffer Ends-----------------------

		// Draw Indexed
		RHICmdList.SetStreamSource(0, MyVertexBufferRHI, 0);		
		RHICmdList.DrawIndexedPrimitive(MyIndexBufferRHI, 0, 0, 4, 0, 2, 1);

		MyIndexBufferRHI.SafeRelease();
		MyVertexBufferRHI.SafeRelease();
	}
	RHICmdList.EndRenderPass();
	RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::RTV, ERHIAccess::SRVMask));
}


void UUtilityFunctions::DrawToQuad(class UTextureRenderTarget2D* OutputRenderTarget, FLinearColor MyColor, UTexture2D* MyTexture)
{
	check(IsInGameThread());

	if (!OutputRenderTarget)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	FRHITexture2D* MyRHITexture2D = MyTexture->TextureReference.TextureReferenceRHI->GetReferencedTexture()->GetTexture2D();


	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, MyColor, MyRHITexture2D](FRHICommandListImmediate& RHICmdList)
		{
			DrawToQuad_RenderThread(RHICmdList, TextureRenderTargetResource, MyColor, MyRHITexture2D);
		}
	);
}

static void UseComputeShader_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* TextureRenderTargetResource
)
{
	check(IsInRenderingThread());

	FTexture2DRHIRef RenderTargetTexture = TextureRenderTargetResource->GetRenderTargetTexture();
	uint32 GroupSize = 32;
	uint32 SizeX = RenderTargetTexture->GetSizeX();
	uint32 SizeY = RenderTargetTexture->GetSizeY();

	FIntPoint FullResolution = FIntPoint(SizeX,SizeY);
	uint32 GroupSizeX = FMath::DivideAndRoundUp((uint32)SizeX, GroupSize);
	uint32 GroupSizeY = FMath::DivideAndRoundUp((uint32)SizeY, GroupSize);

	TShaderMapRef<FMyComputeShader>ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());

	//创建一个贴图资源
	FRHIResourceCreateInfo CreateInfo;
	FTexture2DRHIRef CreatedRHITexture = RHICreateTexture2D(SizeX, SizeY, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	//创建贴图资源的UAV视图
	FUnorderedAccessViewRHIRef TextureUAV = RHICreateUnorderedAccessView(CreatedRHITexture);

	// 将参数传递给ComputeShader
	//这里我们实际上能用到的是UAV,查看SetParameters函数我们可以发现，对于ComputeShader，第二个参数实际上是没有用的
	ComputeShader->SetParameters(RHICmdList, CreatedRHITexture, TextureUAV);
	
	RHICmdList.Transition(FRHITransitionInfo(TextureUAV, ERHIAccess::Unknown, ERHIAccess::UAVMask));
	DispatchComputeShader(RHICmdList, ComputeShader, GroupSizeX, GroupSizeY, 1);

	//把CS输出的UAV贴图拷贝到RenderTargetTexture
	RHICmdList.CopyTexture(CreatedRHITexture, RenderTargetTexture, FRHICopyTextureInfo());
}

void UUtilityFunctions::UseComputeShader(class UTextureRenderTarget2D* OutputRenderTarget)
{
	check(IsInGameThread());

	if (!OutputRenderTarget)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[TextureRenderTargetResource](FRHICommandListImmediate& RHICmdList)
		{
			UseComputeShader_RenderThread
			(
				RHICmdList,
				TextureRenderTargetResource
			);
		}
	);
}

