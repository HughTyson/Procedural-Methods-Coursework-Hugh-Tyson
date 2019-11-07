#pragma once
#include "PlaneMesh.h"
#include "CPerlinNoise.h"
class TerrainMesh :
	public PlaneMesh {
public:
	TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128 );
	~TerrainMesh();

	void Resize( int newResolution );
	void Regenerate( ID3D11Device* device, ID3D11DeviceContext* deviceContext );

	const inline int GetResolution(){ return resolution; }

	void smoothing(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void Generate_Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void FaultLine(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void PerlinNoise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float amplitude, float frequency);

	void BrownianMotion(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int octaves, float frequency, float ampitude);

	void flatten(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void ThermalErosion(ID3D11Device * device, ID3D11DeviceContext * deviceContext, int erosionIterations);

	void HydraulicErosion(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float carryingCapacity, float depositionSpeed, int iterations, int drops, float persistence);

	int ClampNumber(int number, int clamp_value, int min_clamp_value);

private:
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );
	void BuildHeightMap();

	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 100.0f;		//What is the width and height of our terrain
	float* heightMap;

	XMFLOAT3 CrossProd(XMFLOAT3 line, XMFLOAT3 point);
};
