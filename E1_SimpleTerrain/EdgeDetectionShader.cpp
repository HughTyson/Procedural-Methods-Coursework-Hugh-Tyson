#include "EdgeDetectionShader.h"



EdgeDetectionShader::EdgeDetectionShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"EdgeDetection_vs.cso", L"EdgeDetection_ps.cso");
}


EdgeDetectionShader::~EdgeDetectionShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}
	if (screenSizeBuffer)
	{
		screenSizeBuffer->Release();
		screenSizeBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void EdgeDetectionShader::setShaderParameters(ID3D11DeviceContext * deviceContext, const XMMATRIX & worldMatrix, const XMMATRIX & viewMatrix, const XMMATRIX & projectionMatrix, ID3D11ShaderResourceView * texture, float width, float height)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//Additional
	// Send light data to pixel shader
	ScreenSizeBufferType* widthPtr;
	deviceContext->Map(screenSizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	widthPtr = (ScreenSizeBufferType*)mappedResource.pData;
	widthPtr->screenWidth = width;
	widthPtr->screenHeight = height;
	widthPtr->padding = XMFLOAT2(1.0f, 1.f);
	deviceContext->Unmap(screenSizeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &screenSizeBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

void EdgeDetectionShader::initShader(const wchar_t * vsFilename, const wchar_t * psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC screenSizeBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Setup the description of the screen size.
	screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
	screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	screenSizeBufferDesc.MiscFlags = 0;
	screenSizeBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&screenSizeBufferDesc, NULL, &screenSizeBuffer);

}

