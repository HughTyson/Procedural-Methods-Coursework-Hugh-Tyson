#pragma once
#include "PlaneMesh.h"
#include "CPerlinNoise.h"
#include <vector>
#include <math.h>
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

	void ParticleDeposition(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int particles, bool inverse);

	void PerlinNoise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float amplitude, float frequency);

	void BrownianMotion(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int octaves, float frequency, float ampitude);

	void flatten(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void ThermalErosion(ID3D11Device * device, ID3D11DeviceContext * deviceContext, int erosionIterations);

	void Terrace(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float octaves);

	void RigidNoise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float frequency, float amplitude);

	void InverseRigidNoise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float frequency, float amplitude);

	void Redistribution(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float pow);


	
private:
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );
	void BuildHeightMap();

	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 100.0f;		//What is the width and height of our terrain
	float* heightMap;

	XMFLOAT3 CrossProd(XMFLOAT3 line, XMFLOAT3 point);
};
