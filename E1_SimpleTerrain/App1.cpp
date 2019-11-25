// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{
	m_Terrain = nullptr;
	shader = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	textureMgr->loadTexture(L"grass", L"res/Grass.png");
	textureMgr->loadTexture(L"rock", L"res/rock.png");
	textureMgr->loadTexture(L"dirt", L"res/dirtColour.jpg");

	textureMgr->loadTexture(L"red", L"res/redColour.jpg");
	textureMgr->loadTexture(L"black", L"res/blackColour.jpg");
	textureMgr->loadTexture(L"white", L"res/whiteColour.png");

	// Create Mesh object and shader object
	m_Terrain = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext());
	sky_dome = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	shader = new LightShader(renderer->getDevice(), hwnd);
	texture_shader = new TextureShader(renderer->getDevice(), hwnd);

	light = new Light;
	light->setAmbientColour( 0.25f, 0.25f, 0.25f, 1.0f );
	light->setDiffuseColour(0.75f, 0.75f, 0.75f, 1.0f);
	light->setDirection(1.0f,-0.0f, 0.0f);

	camera->setPosition(0.0f, 45.0f, -30.0f);
	camera->setRotation( 0.0f, 30.0f,30.0f );

	terrainResolution = m_Terrain->GetResolution();

	//render textures
	sceneTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	postProcessing = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size

	//post processing
	postPro = new PostProcessing(renderer, hwnd, screenHeight, screenWidth, 3);
	edge_detect = false;
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if ( m_Terrain )
	{
		delete m_Terrain;
		m_Terrain = 0;
	}
	if (shader)
	{
		delete shader;
		shader = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

void App1::firstPass()
{
	if (edge_detect)
	{
		sceneTexture->setRenderTarget(renderer->getDeviceContext());
		sceneTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.3f, 1.0f);
	}
	else
	{
		renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	}

	// Generate the view matrix based on the camera's position.
	camera->update();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	m_Terrain->sendData(renderer->getDeviceContext());

	if (use_colours)
	{
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"white"), textureMgr->getTexture(L"red"), textureMgr->getTexture(L"black"), light);
	}
	else if (!use_colours)
	{
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"rock"), textureMgr->getTexture(L"dirt"), light);
	}


	shader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());

	if (edge_detect)
	{
		renderer->setBackBufferRenderTarget();
	}

}

void App1::PostProcessPass()
{
	if (edge_detect)
	{
		postProcessing = postPro->ApplyEdgeDetectionPostProcessing(renderer, sceneTexture, edge_detect, camera);
	}
}

void App1::renderPass()
{

	if (edge_detect) //only renders to texture if glow is applied
	{
		renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

		// RENDER THE RENDER TEXTURE SCENE
		// Requires 2D rendering and an ortho mesh.
		renderer->setZBuffer(false);
		XMMATRIX worldMatrix = renderer->getWorldMatrix();
		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

		orthoMesh->sendData(renderer->getDeviceContext());
		texture_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, postProcessing->getShaderResourceView());
		texture_shader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		renderer->setZBuffer(true);
	}

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();
}



bool App1::render()
{

	firstPass();

	PostProcessPass();

	renderPass();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text( "Camera Pos: (%.2f, %.2f, %.2f)", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z );
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::SliderInt( "Terrain Resolution", &terrainResolution, 2, 1024 );
	if( ImGui::Button( "Regenerate Terrain" ) ) {
		if( terrainResolution != m_Terrain->GetResolution() ) {
			m_Terrain->Resize( terrainResolution );
		}
		m_Terrain->Regenerate( renderer->getDevice(), renderer->getDeviceContext() );
	}

	ImGui::Text("");

	if (ImGui::Button("Flatten"))
	{
		m_Terrain->flatten(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Smooth"))
	{
		m_Terrain->smoothing(renderer->getDevice(), renderer->getDeviceContext());
	}

	if (ImGui::Button("Faultline"))
	{
		m_Terrain->FaultLine(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Text("");

	ImGui::Text("Functions that use Perlin Noise");

	ImGui::SliderFloat("Frequency", &frequency, 0.01, 0.5);
	ImGui::SliderInt("Amplitude", &amplitude, 5, 45);

	ImGui::Text("");

	if (ImGui::Button("Perlin"))
	{
		m_Terrain->PerlinNoise(renderer->getDevice(), renderer->getDeviceContext(),amplitude,frequency);
	}

	if (ImGui::Button("Rigid Noise"))
	{	
		m_Terrain->RigidNoise(renderer->getDevice(), renderer->getDeviceContext(), frequency, amplitude);	
	}

	if (ImGui::Button("Inverse Rigid Noise"))
	{

		m_Terrain->InverseRigidNoise(renderer->getDevice(), renderer->getDeviceContext(), frequency, amplitude);

	}

	ImGui::Text("");

	ImGui::SliderInt("Terracing Octaves", &terracing_octaves, 1, 100);

	if (ImGui::Button("Terracing Effect"))
	{
		m_Terrain->Terrace(renderer->getDevice(), renderer->getDeviceContext(), terracing_octaves, frequency, amplitude);
	}

	ImGui::Text("");

	ImGui::SliderInt("Brownian Octaves", &brownian_octaves, 1, 30);
	
	if (ImGui::Button("Brownian"))
	{
		m_Terrain->BrownianMotion(renderer->getDevice(), renderer->getDeviceContext(), brownian_octaves, frequency, amplitude);
	}

	ImGui::Text("");

	ImGui::SliderFloat("Power", &power, 0.1, 8);
	if (ImGui::Button("Valley"))
	{
		m_Terrain->Redistribution(renderer->getDevice(), renderer->getDeviceContext(), power,frequency, amplitude);
	}

	ImGui::Text("");

	ImGui::Text("Erosion");

	if (ImGui::Button("Thermal"))
	{
		m_Terrain->ThermalErosion(renderer->getDevice(), renderer->getDeviceContext(), 1);
	}

	ImGui::Text("");
	ImGui::Checkbox("Edge Detection", &edge_detect);	
	
	ImGui::Text("");
	ImGui::Checkbox("Use Colour", &use_colours);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


}

