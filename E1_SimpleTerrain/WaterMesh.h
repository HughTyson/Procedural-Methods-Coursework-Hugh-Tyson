#pragma once
#include "PlaneMesh.h"
#include "CPerlinNoise.h"
class WaterMesh :
	public PlaneMesh
{
public:
	WaterMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128);
	~WaterMesh();

	void Resize(int newResolution);
	void Regenerate(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	const inline int GetResolution() { return resolution; }

	void PerlinNoise3D(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float amplitude, float frequency, float time);
	
private:

	void CreateBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices);
	void BuildHeightMap();
	void Generate_Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 100.0f;		//What is the width and height of our terrain
	float* heightMap;
};

