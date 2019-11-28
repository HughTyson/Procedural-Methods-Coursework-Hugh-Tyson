// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "TerrainMesh.h"
#include "PostProcessing.h"
#include "TextureShader.h"
#include "WaterMesh.h"
#include "WaterShader.h"

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
	WaterShader* water_shader;
	TerrainMesh* m_Terrain;
	WaterMesh* m_Water;
	SphereMesh* sky_dome;

	Light* light;

	//terrain variables

	bool enable_terrain = true;;

	int terrainResolution = 128;

	int brownian_octaves = 10;

	bool use_rigidNoise = false;
	bool use_inverserigidNoise = false;

	float terracing_octaves = 0.4;
	bool use_Terracing = false;

	bool use_capping = false;

	float power = 0;

	//variables for post processing

	RenderTexture* sceneTexture;
	RenderTexture* postProcessing;
	OrthoMesh* orthoMesh;

	PostProcessing* postPro;

	bool edge_detect;

	TextureShader* texture_shader;

	bool use_colours = false;

	//water variables

	bool enable_water;
	float water_frequency;
	float water_amplitude;
	float dt;
	float speed;
	float water_height;
	float transparent_value;

};

#endif