#include "../Shared/pch.h"
#include "../Shared/Timer.h"
#include "../D3DGraphics/Graphics.h"
#include "../Input/Input.h"
#include <time.h>
#include "../Window/Window.h"
#include "../EntityComponent/StaticMesh.h"
#include "../EntityComponent/ComponentPool.h"
#include "../EntityComponent/Transform.h"
#include "../EntityComponent/Entity.h"
#include "../D3DGraphics/ResourcePool.h"
#include "../D3DGraphics/GraphicsComponent.h"
#include "../EntityComponent/CameraController.h"
#include "../EntityComponent/Camera.h"
#include "../EntityComponent/Light.h"
#include "../D3DGraphics/GraphicsDefine.h"
#include "MathDefine.h"
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

Window* wnd = nullptr;
D3DGraphics::D3DGraphics* gfx = nullptr;
MSG msg = {};
bool resizeing = false;
bool maximized = false;
bool minimized = false;
float timeScale = 1;
void Initialize();
bool Tick();
void Shutdown();
void Stats();

int WinMain(HINSTANCE hInstance, HINSTANCE prevhInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Initialize();
	while (Tick()) {};
	Shutdown();
	return 0;
}

LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ENTERSIZEMOVE:
		resizeing = true;
		timeScale = g_Timer.GetTimeScale();
		g_Timer.SetTimeScale(0);
		break;
	case WM_EXITSIZEMOVE:
		resizeing = false;
		g_Timer.SetTimeScale(timeScale);
		break;
	case WM_SIZE:
		wnd->OnResize(LOWORD(lParam), HIWORD(lParam));
		if(EntityComponent::Camera::Main())
			EntityComponent::Camera::Main()->OnResize(LOWORD(lParam), HIWORD(lParam));
		if (gfx->CheckDevice())
		{
			if (wParam == SIZE_MINIMIZED)
			{
				maximized = false;
				minimized = true;
				timeScale = g_Timer.GetTimeScale();
				g_Timer.SetTimeScale(0);
			}
			else if(wParam == SIZE_MAXIMIZED)
			{
				maximized = true;
				minimized = false;
				gfx->OnResize();
				g_Timer.SetTimeScale(timeScale);
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (minimized)
				{
					minimized = false;
					gfx->OnResize();
					g_Timer.SetTimeScale(timeScale);
				}
				else if (maximized)
				{
					maximized = false;
					gfx->OnResize();
					g_Timer.SetTimeScale(timeScale);
				}
			}
			else if (resizeing)
			{

			}
			else
			{
				gfx->OnResize();
			}
		}
		break;
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 800;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 600;
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
using namespace EntityComponent;
#define NUM_LIGHTS 16
Entity* entity[100];
Entity* ground;
Entity* camera;
Entity*	lights[NUM_LIGHTS];
Light* sun;
void Initialize()
{
	srand((unsigned)time(0));
	wnd = new Window;
	wnd->Title = L"MainApp";
	wnd->Width = 1280;
	wnd->Height = 720;
	wnd->SetWindowMode(true);
	gfx = new D3DGraphics::Graphics;
	wnd->Initialize(WndProc);
	Input::Initialize(wnd->Handle());
	gfx->Initialize(wnd);
	ComponentPool::Initialize();
	TexturePool::Initialize();
	
	for (int y = 0; y < 10; ++y)
	{
		int row = y * 10;
		for (int i = 0; i < 10; ++i)
		{
			UINT s = (UINT)(rand() % 5);
			if (s > 3)continue;
			int idx = row + i;
			entity[idx] = ComponentPool::SpawnEntity();
			entity[idx]->GetTransform()->SetWorldPosition(float3((float)i - 5, 0.5f, (float)y - 5));
			StaticMesh* mesh = entity[idx]->AddComponent<StaticMesh>();
			mesh->mesh = MeshPool::meshes[L"Primitive"];
			switch (s)
			{
			case 0:
				mesh->subMesh = &mesh->mesh->DrawArgs[L"Box"];
				break;
			case 1:
				mesh->subMesh = &mesh->mesh->DrawArgs[L"Cylinder"];
				break;
			case 2:
				mesh->subMesh = &mesh->mesh->DrawArgs[L"Sphere"];
				break;
			case 3:
				mesh->subMesh = &mesh->mesh->DrawArgs[L"GeoSphere"];
				break;
			default:
				mesh->subMesh = &mesh->mesh->DrawArgs[L"Plane"];
				break;
			}
			BoundingOrientedBox::CreateFromBoundingBox(mesh->boundingBox, mesh->subMesh->Bound);
			mesh->boundingBox.Center = entity[idx]->GetTransform()->WorldPosition();
			mesh->boundingBox.Orientation = entity[idx]->GetTransform()->WorldRotation();
			XMVECTOR ext = XMLoadFloat3(&mesh->boundingBox.Extents) * entity[idx]->GetTransform()->GetWorldScale();
			XMStoreFloat3(&mesh->boundingBox.Extents, ext);
		}
	}
	//entity[0]->GetTransform()->SetUp(0.3f, 1, -0.7f);
	
	//light->SetEnable(0);
	ground = Spawn();
	ground->GetTransform()->SetWorldScale(25, 1, 25);
	StaticMesh* groundMesh = ground->AddComponent<StaticMesh>();
	groundMesh->mesh = MeshPool::meshes[L"Primitive"];
	groundMesh->subMesh = &groundMesh->mesh->DrawArgs[L"Plane"];
	BoundingOrientedBox::CreateFromBoundingBox(groundMesh->boundingBox, groundMesh->subMesh->Bound);

	groundMesh->boundingBox.Center = ground->GetTransform()->WorldPosition();
	groundMesh->boundingBox.Orientation = ground->GetTransform()->WorldRotation();
	XMVECTOR ext = XMLoadFloat3(&groundMesh->boundingBox.Extents) * ground->GetTransform()->GetWorldScale();
	XMStoreFloat3(&groundMesh->boundingBox.Extents, ext);

	camera = ComponentPool::SpawnEntity({ -0.315693f, 6.04092f, -9.25411f }, { 0.333794f, -0.00822598f, 0.00291298f, 0.942606f });
	//camera->GetTransform()->SetWorldPosition(-0.315693f, 6.04092f, -9.25411f);
	Camera* c = camera->AddComponent<Camera>();
	Light* light;
	lights[0] = Spawn();
	lights[0]->GetTransform()->SetForward(-.3f, -0.2f, 0.4f);
	//XMVECTOR forward = lights[0]->GetTransform()->Forward();
	//lights[0]->GetTransform()->SetWorldPosition(-forward * 35.4f);
	sun = lights[0]->AddComponent<Light>();
	sun->lightType = D3DGraphics::LIGHT_DIRECTIONAL;
	sun->color = { 0.8f,0.79f, 0.61f };
	sun->radius = 100;
	sun->intensity = 1;
	sun->falloffInner = cosf(12 * G_DEG2RAD);
	sun->falloffOutter = cosf(28 * G_DEG2RAD);
	sun->falloffNear = 0.01f;
	sun->falloffFar = sun->radius;
	camera->AddComponent<CameraController>();

	for (int i = 1; i < NUM_LIGHTS; ++i)
	{
		lights[i] = Spawn(
		{ ((float)rand() / RAND_MAX) * 10 - 5, ((float)rand() / RAND_MAX) * 2, ((float)rand() / RAND_MAX) * 10 - 5 }
		);
		lights[i]->GetTransform()->SetForward((float)rand() / RAND_MAX - 0.5f, -(float)rand() / RAND_MAX * 2, (float)rand() / RAND_MAX - 0.5f);
		lights[i]->GetTransform()->SetWorldScale(0.1f, 0.1f, 0.3f);

		light = lights[i]->AddComponent<Light>();
		light->intensity = ((float)rand() / RAND_MAX) * 2 + 0.5f;
		light->color = { (float)rand() / RAND_MAX , (float)rand() / RAND_MAX, (float)rand() / RAND_MAX };
		light->radius = (float)rand() / RAND_MAX * 5 + 0.5f;
		light->lightType = rand() % D3DGraphics::LightType::LIGHT_SPOT + 1;
		light->falloffInner = cosf(((float)rand() / RAND_MAX * 11 + 20) * G_DEG2RAD);
		light->falloffOutter = cosf(((float)rand() / RAND_MAX * 10 + 10) * G_DEG2RAD);
		light->falloffNear = (float)rand() / RAND_MAX * 0.5f;
		light->falloffFar = light->radius;

		/*StaticMesh* mesh = lights[i]->AddComponent<StaticMesh>();
		mesh->mesh = MeshPool::meshes[L"Primitive"];
		switch (light->lightType)
		{
		case D3DGraphics::LIGHT_DIRECTIONAL:
			mesh->subMesh = &mesh->mesh->DrawArgs[L"Cylinder"];
			break;
		case D3DGraphics::LIGHT_POINT:
			mesh->subMesh = &mesh->mesh->DrawArgs[L"Sphere"];
			break;
		case D3DGraphics::LIGHT_SPOT:
			mesh->subMesh = &mesh->mesh->DrawArgs[L"Box"];
			break;
		}*/
	}

	c->Initialize(90.0f, (float)wnd->Width, (float)wnd->Height, 0.1f, 100.0f);
	g_Timer.Reset();
	gfx->PostInitialize();
	MeshPool::Disposal();
	TexturePool::Disposal();
}

bool Tick()
{
	g_Timer.Tick();
	Input::UpdateInput();

	if(Input::IsKeyHold(Input::KeyCode::Right))
		lights[0]->GetTransform()->RotateLocal(0, g_Timer.DeltaTime() , 0);
	if (Input::IsKeyHold(Input::KeyCode::Left))
		lights[0]->GetTransform()->RotateLocal(0, -g_Timer.DeltaTime(), 0);
	if (Input::IsKeyHold(Input::KeyCode::Up))
		lights[0]->GetTransform()->Rotate(g_Timer.DeltaTime(), 0, 0);
	if (Input::IsKeyHold(Input::KeyCode::Down))
		lights[0]->GetTransform()->Rotate(-g_Timer.DeltaTime(), 0, 0);
	if (Input::IsKeyHold(Input::KeyCode::Add))
		sun->intensity += g_Timer.DeltaTime();
	if (Input::IsKeyHold(Input::KeyCode::Subtract))
		sun->intensity -= g_Timer.DeltaTime();
	ComponentPool::Tick();
	gfx->Tick();
	gfx->Render();

	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT) return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	Stats();
	return true;
}

void Shutdown()
{
	delete wnd;
	wnd = nullptr;
	delete gfx;
	gfx = nullptr;
	MeshPool::Shutdown();
}

void Stats()
{
	static int frameCount = 0;
	static double totalTime = 0;
	++frameCount;
	std::wstring stats;
	if (g_Timer.Elapsed() - totalTime >= 1.0f)
	{
		stats = wnd->Title;
		stats += L" fps: ";
		stats += std::to_wstring(frameCount) + L" ms: ";
		stats += std::to_wstring(1 / (float)frameCount);
		frameCount = 0;
		totalTime += 1.0f;
		SetWindowText(wnd->Handle(), stats.c_str());
	}
}

