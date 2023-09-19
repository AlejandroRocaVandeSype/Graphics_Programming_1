//External includes
#include "vld.h"
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"RayTracer - Alejandro Roca Vande Sype",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	const auto pScene = new Scene_W1();
	pScene->Initialize();

	//Start loop
	pTimer->Start();

	// Start Benchmark
	// pTimer->StartBenchmark();

	//// Testing Dot and Cross products
	//Vector3 v1{ 1, 0, 0 };
	//Vector3 v2{ 0, 2, 2 };
	//Vector3 v3{ -1, 2, 1 };

	//std::cout << Vector3::Dot(v1,v2) << std::endl;
	//std::cout << Vector3::Dot(v2, v1) << std::endl;
	//std::cout << Vector3::Dot(v1, v3) << std::endl;
	//std::cout << Vector3::Dot(v2, v3) << std::endl;
	//std::cout << Vector3::Dot(Vector3::UnitX, Vector3::UnitX) << std::endl; // (1) Same direction
	//std::cout << Vector3::Dot(Vector3::UnitX, -Vector3::UnitX) << std::endl; // (-1) Opposite Direction
	//std::cout << Vector3::Dot(Vector3::UnitX, Vector3::UnitY) << std::endl; // (0) Perpendicular

	//Vector3 crossResult{};
	//std::cout << "Cross products :";
	//crossResult = Vector3::Cross(Vector3::UnitZ, Vector3::UnitX);
	//std::cout << "(" << crossResult.x << ", " << crossResult.y << ", " << crossResult.z <<")" << std::endl;	// Perpendicular vector for both (0, 1, 0)
	//
	//crossResult = Vector3::Cross(Vector3::UnitX, Vector3::UnitZ);
	//std::cout << "(" << crossResult.x << ", " << crossResult.y << ", " << crossResult.z << ")" << std::endl; // (0, -1, 0)

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if(e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;
				break;
			}
		}

		//--------- Update ---------
		pScene->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render(pScene);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pScene;
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}