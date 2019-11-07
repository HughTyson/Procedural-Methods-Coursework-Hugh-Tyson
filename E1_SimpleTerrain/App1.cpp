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
	textureMgr->loadTexture(L"dirt", L"res/dirt.png");

	// Create Mesh object and shader object
	m_Terrain = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext());
	shader = new LightShader(renderer->getDevice(), hwnd);

	light = new Light;
	light->setAmbientColour( 0.25f, 0.25f, 0.25f, 1.0f );
	light->setDiffuseColour(0.75f, 0.75f, 0.75f, 1.0f);
	light->setDirection(1.0f,-0.0f, 0.0f);

	camera->setPosition(0.0f, 45.0f, -30.0f);
	camera->setRotation( 0.0f, 30.0f,30.0f );

	terrainResolution = m_Terrain->GetResolution();
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

bool App1::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	m_Terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"rock"), textureMgr->getTexture(L"dirt"), light);
	shader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());
	
	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

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

	ImGui::SliderFloat("Frequency", &frequency, 0.01, 0.5);
	ImGui::SliderInt("Amplitude", &amplitude, 5, 45);

	if (ImGui::Button("Perlin"))
	{
		m_Terrain->PerlinNoise(renderer->getDevice(), renderer->getDeviceContext(),amplitude,frequency);
	}

	
	if (ImGui::Button("Brownian"))
	{
		m_Terrain->BrownianMotion(renderer->getDevice(), renderer->getDeviceContext(), 15, frequency, amplitude);
	}

	if (ImGui::Button("Thermal"))
	{
		m_Terrain->ThermalErosion(renderer->getDevice(), renderer->getDeviceContext(), 1);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

