#include "TerrainMesh.h"

#define clamp(value,minimum,maximum) max(min(value,maximum),minimum)
TerrainMesh::TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution ) :
	PlaneMesh( device, deviceContext, lresolution ) 
{
	Resize( resolution );
	Regenerate( device, deviceContext );
}

//Cleanup the heightMap
TerrainMesh::~TerrainMesh() {
	delete[] heightMap;
	heightMap = 0;
}


//Fill an array of floats that represent the height values at each grid point.
//Here we are producing a Sine wave along the X-axis
void TerrainMesh::BuildHeightMap() {
	float height = 0.0f;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale =  terrainSize / (float)resolution;

	//TODO: Give some meaning to these magic numbers! What effect does changing them have on terrain?
	for( int j = 0; j < ( resolution ); j++ ) {
		for( int i = 0; i < ( resolution ); i++ ) {
			height = 1;
			heightMap[( j * resolution ) + i] = height;
		}
	}	
}

void TerrainMesh::Resize( int newResolution ) {
	resolution = newResolution;
	heightMap = new float[resolution * resolution];
	if( vertexBuffer != NULL ) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;
}

// Set up the heightmap and create or update the appropriate buffers
void TerrainMesh::Regenerate( ID3D11Device * device, ID3D11DeviceContext * deviceContext ) {

	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	
	//Calculate and store the height values
	BuildHeightMap();

	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ( ( resolution - 1 ) * ( resolution - 1 ) ) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = m_UVscale / resolution;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//Set up vertices
	for( j = 0; j < ( resolution ); j++ ) {
		for( i = 0; i < ( resolution ); i++ ) {
			positionX = (float)i * scale;
			positionZ = (float)( j ) * scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3( positionX, height, positionZ );
			vertices[index].texture = XMFLOAT2( u, v );

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {

			//Build index array
			indices[index] = ( j*resolution ) + i;
			indices[index + 1] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			indices[index + 2] = ( ( j + 1 ) * resolution ) + i;

			indices[index + 3] = ( j * resolution ) + i;
			indices[index + 4] = ( j * resolution ) + ( i + 1 );
			indices[index + 5] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			index += 6;
		}
	}

	//Set up normals
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[( j + 1 ) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab( c.x - a.x, c.y - a.y, c.z - a.z );
			XMFLOAT3 ac( b.x - a.x, b.y - a.y, b.z - a.z );
			
			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = ( cross.x * cross.x ) + ( cross.y * cross.y ) + ( cross.z * cross.z );
			mag = sqrtf( mag );
			cross.x/= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal( 0, 1, 0 );
	for( j = 0; j < resolution; j++ ) {
		for( i = 0; i < resolution; i++ ) {
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;
			//Left planes
			if( ( i - 1 ) >= 0 ) {
				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[j * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[j * resolution + ( i - 1 )].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.z;
					count++;
				}
			}
			//right planes
			if( ( i ) <( resolution - 1 ) ) {

				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + i].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + i].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + i].normal.z;
					count++;
				}
			}
			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt( ( smoothedNormal.x * smoothedNormal.x ) + ( smoothedNormal.y * smoothedNormal.y ) + ( smoothedNormal.z * smoothedNormal.z ) );
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if( vertexBuffer == NULL ) {
		CreateBuffers( device, vertices, indices );
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map( vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		//  Update the vertex buffer here.
		memcpy( mappedResource.pData, vertices, sizeof( VertexType ) * vertexCount );
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap( vertexBuffer, 0 );
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

void TerrainMesh::smoothing(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
{

	float* _copy = heightMap;

	for (int y = 0; y < (resolution); y++)
	{
		for (int x = 0; x < (resolution); x++)
		{

			float average = 0.f;
			int values_used = 0;
			int topL = 0;
			int topR = 0;
			int bottomL = 0;
			int bottomR = 0;

			if (x != 0)
			{
				values_used++;
				average += heightMap[(y*resolution) + (x - 1)];
				bottomL++;
				bottomR++;

			}
			if (x != resolution - 1)
			{
				values_used++;
				average += heightMap[(y*resolution) + (x + 1)];
				topL += 1;
				topR += 1;
			}

			if (y != 0)
			{
				values_used++;
				average += heightMap[(y - 1)*resolution + (x)];
				bottomL++;
				bottomR++;
			}
			if (y != resolution - 1)
			{
				values_used++;
				average += heightMap[(y + 1)*resolution + (x)];
topL += 1;
topR += 1;
			}

			if (topL == 2)
			{
				values_used++;
				average += heightMap[((y + 1)*resolution) + ((x - 1))];
			}
			if (topR == 2)
			{
				values_used++;
				average += heightMap[((y + 1) * resolution) + ((x + 1))];
			}
			if (bottomR == 2)
			{
				values_used++;
				average += heightMap[(y - 1)*resolution + ((x + 1))];
			}
			if (bottomL == 2)
			{
				values_used++;
				average += heightMap[(y - 1)*resolution + ((x - 1))];
			}


			average = average / values_used;

			_copy[(y * resolution) + x] = average;
		}
	}

	heightMap = _copy;

	Generate_Mesh(device, deviceContext);

}

void TerrainMesh::FaultLine(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
{

	float x1;
	float x2;
	float y1;
	float y2;

	//get 4 random points on the terrain

	x1 = (float)(rand() % 10000 / 10000.f) *resolution;
	x2 = (float)(rand() % 10000 / 10000.f) *resolution;

	y1 = (float)(rand() % 10000 / 10000.f) *resolution;
	y2 = (float)(rand() % 10000 / 10000.f) *resolution;

	//create line vector
	XMFLOAT3 line = XMFLOAT3(x2 - x1, 0, y2 - y1);

	//calculate the height increase value using a random value
	float height_increase = 5.f - ((float)(rand() % 100) / 100) *10.f;

	for (int y = 0; y < (resolution); y++) //loop for y 
	{
		for (int x = 0; x < (resolution); x++) //loop for x
		{

			XMFLOAT3 point = XMFLOAT3(x1 - x, 0, y1 - y); //get point on the terrain

			XMFLOAT3 cross = CrossProd(line, point); //cross product between the line and point

			if (cross.y > 0) // if cross value is greater than 0 thne increase the hight by set height increase value
			{
				heightMap[(y * resolution) + x] += height_increase; //add value onto heightmap
			}
		}
	}

	Generate_Mesh(device, deviceContext);

}

void TerrainMesh::ParticleDeposition(ID3D11Device * device, ID3D11DeviceContext * deviceContext, int particles, bool inverse)
{

	int radius = 2;
	int amount_of_points;

	if (radius == 1)
	{
		amount_of_points = 6;
	}
	else if (radius == 2)
	{
		amount_of_points = 23;
	}
	int x_val, y_val;
	int x1, x2, y1, y2;
	float current_height;
	float surrounding_heights[23];

	x_val = rand() % resolution;
	y_val = rand() % resolution;

	for (int i = 0; i < radius; i++)
	{
		x1 = x_val - i;
		x2 = x_val + i;

		y1 = y_val - i;
		y2 = y_val + i;

		clamp(x1, 0, resolution - 1);
		clamp(x2, 0, resolution - 1);
		clamp(y1, 0, resolution - 1);
		clamp(y2, 0, resolution - 1);


		surrounding_heights[0] = heightMap[((x1 * resolution) + y1)];
		surrounding_heights[1] = heightMap[(y_val * resolution) + y1];
		surrounding_heights[2] = heightMap[(x2 * resolution) + y1];
		surrounding_heights[3] = heightMap[(x1 * resolution) + (x_val)];
		surrounding_heights[4] = heightMap[(x2 * resolution) + (x_val)];
		surrounding_heights[5] = heightMap[(x1 * resolution) + y2];
		surrounding_heights[6] = heightMap[((y_val)* resolution) + y2];
		surrounding_heights[7] = heightMap[(x2 * resolution) + y2];
	}

	for (int i = 0; i < particles; i++)
	{

		current_height = heightMap[(y_val*resolution) + x_val];

		if (heightMap[(y_val*resolution) + (x_val + 1)] != current_height)
		{
			heightMap[(y_val*resolution) + (x_val + 1)]++;
		}
		else if (heightMap[(y_val*resolution) + (x_val - 1)] != current_height)
		{
			heightMap[(y_val*resolution) + (x_val - 1)]++;
		}
		else if (heightMap[((y_val + 1)*resolution) + x_val] != current_height)
		{
			heightMap[((y_val + 1)*resolution) + x_val]++;
		}
		else if (heightMap[((y_val - 1)*resolution) + x_val] != current_height)
		{
			heightMap[((y_val - 1)*resolution) + x_val]++;
		}
		else
		{
			heightMap[(y_val*resolution) + x_val] = heightMap[(y_val*resolution) + x_val]++;
		}
	}

	Generate_Mesh(device, deviceContext);
}

void TerrainMesh::PerlinNoise(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float amplitude, float frequency)
{

	const float scale = terrainSize / (float)resolution;

	for (int j = 0; j < (resolution); j++) 
	{
		for (int i = 0; i < (resolution); i++) 
		{
			float test[2] = { (float)i * frequency*scale, (float)j *frequency *scale};

			heightMap[(j * resolution) + i] += CPerlinNoise::noise2(test)*amplitude;

		}
	}

	Generate_Mesh(device, deviceContext);

}

void TerrainMesh::PerlinNoise3D(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float amplitude, float frequency, float time)
{

	const float scale = terrainSize / (float)resolution;

	for (int j = 0; j < (resolution); j++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			float test[3] = { (float)((i * frequency*scale)+ time), (float)j *frequency *scale, (float)time};

			heightMap[(j * resolution) + i] = CPerlinNoise::noise3(test)*amplitude;
		}
	}

	Generate_Mesh(device, deviceContext);
}

void TerrainMesh::BrownianMotion(ID3D11Device * device, ID3D11DeviceContext * deviceContext, int octaves, float frequency, float amplitude)
{

	for (int i = 0; i < octaves; i++) //loop for the amount of octaves for fBm
	{
		
		PerlinNoise(device, deviceContext, amplitude, frequency); //call the perlin noise function with current amplitude and frequency

		amplitude*=0.5; // half the amplitude
		frequency *= 2; // double the frequency
		
	}


}

void TerrainMesh::Terrace(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float octaves, float frequency, float amplitude)
{

	//for (int j = 0; j < (resolution); j++) {
	//	for (int i = 0; i < (resolution); i++) {
	//		heightMap[(j * resolution) + i] = floor(heightMap[(j * resolution) + i])/ (float)octaves;
	//	}
	//}

	float width = octaves;
	float current_height;
	float floor_height;
	float math;
	float more;

	for (int j = 0; j < (resolution); j++) {
		for (int i = 0; i < (resolution); i++) {

			heightMap[(j * resolution) + i] = floor(heightMap[(j * resolution) + i]) / (float)octaves;
		}
	}

	Generate_Mesh(device, deviceContext);
		
}

void TerrainMesh::RigidNoise(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float frequency, float amplitude)
{
	const float scale = terrainSize / (float)resolution;

	for (int j = 0; j < (resolution); j++) // loop for resolution - y
	{
		for (int i = 0; i < (resolution); i++) // loop for resolution - x
		{
			float test[2] = { (float)i * frequency*scale, (float)j *frequency *scale }; //set point to be passed into perlin noise function

			heightMap[(j * resolution) + i] += -(1.f - abs(CPerlinNoise::noise2(test)*amplitude)); //set heightmap point to inverse of 1 subtracted by the absolute(positive) value of perlin noise

		}
	}

	Generate_Mesh(device, deviceContext); //change mesh

}

void TerrainMesh::InverseRigidNoise(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float frequency, float amplitude)
{
	const float scale = terrainSize / (float)resolution;

	for (int j = 0; j < (resolution); j++) // loop for resolution - y
	{
		for (int i = 0; i < (resolution); i++) // loop for resolution - x
		{
			float test[2] = { (float)i * frequency*scale, (float)j *frequency *scale };  //set point to be passed into perlin noise function

			heightMap[(j * resolution) + i] += (1.f - abs(CPerlinNoise::noise2(test)*amplitude)); //set heightmap point to 1 subtracted by the absolute(positive) value of perlin noise

		}
	}

	Generate_Mesh(device, deviceContext);
}

void TerrainMesh::Redistribution(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float power, float frequency, float amplitude)
{

	const float scale = terrainSize / (float)resolution;
	float noise_value;

	for (int j = 0; j < (resolution); j++) 
	{
		for (int i = 0; i < (resolution); i++) 
		{
			noise_value = (abs(heightMap[(j * resolution) + i]));
			heightMap[(j * resolution) + i] = pow((noise_value), power);//*amplitude;
		}
	}
	Generate_Mesh(device, deviceContext);

}

void TerrainMesh::ThermalErosion(ID3D11Device * device, ID3D11DeviceContext * deviceContext, int erosionIterations)
{

	float heightDifference[8];
	float* _copy = heightMap;
	float height;
	float talus = 4/resolution; //angle at which sediment is dropped
	float c = 0.5; //constant value
	float NumberOver = 0.f;

	
	for (int j = 0; j < (resolution); j++) //loop for y
	{
		for (int i = 0; i < (resolution); i++) //loop for x
		{

			//4 variables to check Moore neighbourhood (8 surrounding points)
			int x1 = j - 1; 
			int x2 = j + 1;

			int y1 = i - 1;
			int y2 = i + 1;

			//calmp value between 0 and resolution - 1
			x1 = clamp(x1, 0, resolution - 1);
			x2 = clamp(x2, 0, resolution - 1);
			y1 = clamp(y1, 0, resolution - 1);
			y2 = clamp(y2, 0, resolution - 1);

			for (int l = 0; l < 8; l++)
			{
				heightDifference[l] = 0; //set all height differences to 0
			}

			//get difference between point and surrounding 8
			heightDifference[0] = heightMap[(j * resolution) + i] - heightMap[((x1 * resolution) + y1)];
			heightDifference[1] = heightMap[(j * resolution) + i] - heightMap[(j * resolution) + y1];
			heightDifference[2] = heightMap[(j * resolution) + i] - heightMap[(x2 * resolution) + y1];
			heightDifference[3] = heightMap[(j * resolution) + i] - heightMap[(x1 * resolution) + (i)];
			heightDifference[4] = heightMap[(j * resolution) + i] - heightMap[(x2 * resolution) + (i)];
			heightDifference[5] = heightMap[(j * resolution) + i] - heightMap[(x1 * resolution) + y2];
			heightDifference[6] = heightMap[(j * resolution) + i] - heightMap[((j)* resolution) + y2];
			heightDifference[7] = heightMap[(j * resolution) + i] - heightMap[(x2 * resolution) + y2];

			float max_dif = 0.f;
			float total_dif = 0.f;
			int over = 0;

			for (int z = 0; z < 8; z++) //loop for all 8 surrounding points
			{

				if (heightDifference[z] > talus) //check if difference is greater than angle
				{
					total_dif += heightDifference[z]; //if is increase total difference
					NumberOver++;						//keep track of amount of numbers over
					
					if (heightDifference[z] > max_dif) //find which value is the maximum value
					{
						max_dif = heightDifference[z]; //set max value to new point
					}
				}
			}

			//chekc all surrounded points are within bound of array, if it is then alter the point using equation
			if (total_dif != 0.f) 
			{				
				if (x1 != j && y1 != i && heightDifference[0] != 0.f )
				{
					_copy[(x1*resolution) + y1] += c * (max_dif - talus) * (heightDifference[0] / total_dif);
				}
				if (y1 != i && heightDifference[1] != 0.f )
				{
					_copy[(j*resolution) + y1] += c * (max_dif - talus) * (heightDifference[1] / total_dif);
				}
				if (x1 != j && heightDifference[3] != 0.f)
				{
					_copy[(x1*resolution) + i] += c * (max_dif - talus) * (heightDifference[3] / total_dif);
				}
				if (x2 != j && y1 != i && heightDifference[2] != 0.f)
				{
					_copy[(x2*resolution) + y1] += c * (max_dif - talus) * (heightDifference[2] / total_dif);
				}
				if (x2 != j && heightDifference[4] != 0.f)
				{
					_copy[(x2*resolution) + i] += c * (max_dif - talus) * (heightDifference[4] / total_dif);
				}
				if (x1 != j && y2 != i && heightDifference[5] != 0.f && total_dif != 0.f)
				{
					_copy[(x1*resolution) + y2] += c * (max_dif - talus) * (heightDifference[5] / total_dif);
				}
				if (y1 != i && heightDifference[6] != 0.f)
				{
					_copy[(j*resolution)+y2] += c * (max_dif - talus) * (heightDifference[6] / total_dif);
				}
				if (x2 != j && y2 != i && heightDifference[7] != 0.f )
				{
					_copy[((x2*resolution) + y2)] += c * (max_dif - talus) * (heightDifference[7] / total_dif);
				}
				_copy[(j *resolution) + (i)] += (max_dif - (NumberOver * max_dif * talus / total_dif)); //alter the initial point
				
			}

		}
	}
	
	heightMap = _copy; // set the copy of the heightmap to the 

	Generate_Mesh(device, deviceContext); //generate the new heightmap

}

void TerrainMesh::flatten(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
{
	float height;
	for (int j = 0; j < (resolution); j++) {
		for (int i = 0; i < (resolution); i++) {
			height = 1;
			heightMap[(j * resolution) + i] = height;
		}
	}
	
	Generate_Mesh(device, deviceContext);

}

XMFLOAT3 TerrainMesh::CrossProd(XMFLOAT3 line, XMFLOAT3 point)
{
	XMFLOAT3 cross;
	cross.x = line.y * point.z - line.z * point.y;
	cross.y = line.z * point.x - line.x * point.z;
	cross.z = line.x * point.y - line.y * point.x;

	return cross;
}

void TerrainMesh::Generate_Mesh(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
{

	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ((resolution - 1) * (resolution - 1)) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = m_UVscale / resolution;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//Set up vertices
	for (j = 0; j < (resolution); j++) {
		for (i = 0; i < (resolution); i++) {
			positionX = (float)i * scale;
			positionZ = (float)(j)* scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3(positionX, height, positionZ);
			vertices[index].texture = XMFLOAT2(u, v);

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for (j = 0; j < (resolution - 1); j++) {
		for (i = 0; i < (resolution - 1); i++) {

			//Build index array
			indices[index] = (j*resolution) + i;
			indices[index + 1] = ((j + 1) * resolution) + (i + 1);
			indices[index + 2] = ((j + 1) * resolution) + i;

			indices[index + 3] = (j * resolution) + i;
			indices[index + 4] = (j * resolution) + (i + 1);
			indices[index + 5] = ((j + 1) * resolution) + (i + 1);
			index += 6;
		}
	}

	//Set up normals
	for (j = 0; j < (resolution - 1); j++) {
		for (i = 0; i < (resolution - 1); i++) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[(j + 1) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab(c.x - a.x, c.y - a.y, c.z - a.z);
			XMFLOAT3 ac(b.x - a.x, b.y - a.y, b.z - a.z);

			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = (cross.x * cross.x) + (cross.y * cross.y) + (cross.z * cross.z);
			mag = sqrtf(mag);
			cross.x /= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal(0, 1, 0);
	for (j = 0; j < resolution; j++) {
		for (i = 0; i < resolution; i++) {
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;
			//Left planes
			if ((i - 1) >= 0) {
				//Top planes
				if ((j) < (resolution - 1)) {
					smoothedNormal.x += vertices[j * resolution + (i - 1)].normal.x;
					smoothedNormal.y += vertices[j * resolution + (i - 1)].normal.y;
					smoothedNormal.z += vertices[j * resolution + (i - 1)].normal.z;
					count++;
				}
				//Bottom planes
				if ((j - 1) >= 0) {
					smoothedNormal.x += vertices[(j - 1) * resolution + (i - 1)].normal.x;
					smoothedNormal.y += vertices[(j - 1) * resolution + (i - 1)].normal.y;
					smoothedNormal.z += vertices[(j - 1) * resolution + (i - 1)].normal.z;
					count++;
				}
			}
			//right planes
			if ((i) <(resolution - 1)) {

				//Top planes
				if ((j) < (resolution - 1)) {
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}
				//Bottom planes
				if ((j - 1) >= 0) {
					smoothedNormal.x += vertices[(j - 1) * resolution + i].normal.x;
					smoothedNormal.y += vertices[(j - 1) * resolution + i].normal.y;
					smoothedNormal.z += vertices[(j - 1) * resolution + i].normal.z;
					count++;
				}
			}
			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt((smoothedNormal.x * smoothedNormal.x) + (smoothedNormal.y * smoothedNormal.y) + (smoothedNormal.z * smoothedNormal.z));
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if (vertexBuffer == NULL) {
		CreateBuffers(device, vertices, indices);
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//  Update the vertex buffer here.
		memcpy(mappedResource.pData, vertices, sizeof(VertexType) * vertexCount);
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap(vertexBuffer, 0);
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}



//Create the vertex and index buffers that will be passed along to the graphics card for rendering
//For CMP305, you don't need to worry so much about how or why yet, but notice the Vertex buffer is DYNAMIC here as we are changing the values often
void TerrainMesh::CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices ) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof( VertexType ) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer( &vertexBufferDesc, &vertexData, &vertexBuffer );

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( unsigned long ) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer( &indexBufferDesc, &indexData, &indexBuffer );
}