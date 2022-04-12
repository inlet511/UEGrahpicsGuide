#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilityFunctions.generated.h"

UCLASS(MinimalAPI)
class UUtilityFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "KenUtility", meta = (WorldContext = "WorldContexObject"))
	static void DrawToQuad(class UTextureRenderTarget2D* OutputRenderTarget,FLinearColor MyColor, UTexture2D* MyTexture);

	UFUNCTION(BlueprintCallable, Category = "KenUtility")
	static void UseComputeShader(class UTextureRenderTarget2D* OutputRenderTarget);

	//UFUNCTION(BlueprintCallable, Category = "KenUtility")
	//static void UseStructuredBuffer();
};