#pragma once

#include "DXF.h"
#include "TextureShader.h"
#include "EdgeDetectionShader.h"

class PostProcessing
{
public:
	PostProcessing(D3D* renderer, HWND hwnd, int screenHeight, int screenWidth, int downscaleAmount);
	~PostProcessing();

	RenderTexture* ApplyEdgeDetectionPostProcessing(D3D* renderer, RenderTexture* curretnRenderTexture, bool edge_detect, Camera* camera);

	void EdgeDetection(D3D* renderer, RenderTexture* currentTexture, Camera* camera);

private:

	//Render Textures

	//Blur Render Texture
	RenderTexture* horizontalBlurTex;
	RenderTexture* verticleBlurTex;
	RenderTexture* downScaleTex;
	RenderTexture* upScaleTex;

	//Glow render Textures
	RenderTexture* glowCalcTex;
	RenderTexture* glowTex;
	
	//Edge Detection

	RenderTexture* edgeDetectionTex;

	//orthoMesh

	OrthoMesh* orthoMesh;
	
	//Shaders

	TextureShader* textureShader;
	EdgeDetectionShader* edgeShader;

};

