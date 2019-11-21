#include "PostProcessing.h"

PostProcessing::PostProcessing(D3D* renderer, HWND hwnd, int screenHeight, int screenWidth, int downscaleAmount)
{

	//ortho initialiser
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	
	
	
	edgeDetectionTex = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	edgeShader = new EdgeDetectionShader(renderer->getDevice(), hwnd);
}

PostProcessing::~PostProcessing()
{
}

RenderTexture * PostProcessing::ApplyEdgeDetectionPostProcessing(D3D* renderer, RenderTexture * currentRenderTexture, bool edge_detect, Camera* camera)
{	
	if (edge_detect)
	{
		EdgeDetection(renderer, currentRenderTexture, camera);

		return edgeDetectionTex;
	}

	return currentRenderTexture;
}

void PostProcessing::EdgeDetection(D3D * renderer, RenderTexture* currentTexture, Camera* camera)
{

	edgeDetectionTex->setRenderTarget(renderer->getDeviceContext());
	edgeDetectionTex->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

	float screenHeight = (float)edgeDetectionTex->getTextureHeight();
	float screenWidth = (float)edgeDetectionTex->getTextureWidth();
	
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = edgeDetectionTex->getOrthoMatrix();

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	edgeShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, currentTexture->getShaderResourceView(), screenWidth, screenHeight);
	edgeShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	//shader stuff

	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();

}



