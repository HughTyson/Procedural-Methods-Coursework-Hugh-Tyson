// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "TerrainMesh.h"
#include "PostProcessing.h"
#include "TextureShader.h"

class App1 : public BaseApplication
{
public:
	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

	void firstPass();

	void PostProcessPass();

	void renderPass();

protected:
	bool render();
	void gui();

	float frequency = 0.12;
	int amplitude = 10;

private:
	LightShader* shader;
	TerrainMesh* m_Terrain;
	SphereMesh* sky_dome;

	Light* light;

	int terrainResolution = 128;

	int brownian_octaves = 10;

	bool use_rigidNoise = false;

	//variables for post processing

	RenderTexture* sceneTexture;
	RenderTexture* postProcessing;
	OrthoMesh* orthoMesh;

	PostProcessing* postPro;

	bool edge_detect;

	TextureShader* texture_shader;

};

#endif