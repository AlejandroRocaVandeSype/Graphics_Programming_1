#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		//Vector3 forward{0.266f, -0.453f, 0.860f};
		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		// Returns the Camera ONB matrix
		Matrix CalculateCameraToWorld()
		{
			// We calculate the Right & up vector using the forward camera vector
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			// Combine into a matrix including the origin
			// RIGHT
			cameraToWorld[0][0] = right.x;
			cameraToWorld[0][1] = right.y;
			cameraToWorld[0][2] = right.z;

			// UP
			cameraToWorld[1][0] = up.x;
			cameraToWorld[1][1] = up.y;
			cameraToWorld[1][2] = up.z;

			// Forward
			cameraToWorld[2][0] = forward.x;
			cameraToWorld[2][1] = forward.y;
			cameraToWorld[2][2] = forward.z;

			// Set camera origin as the translation
			cameraToWorld[3][0] = origin.x;
			cameraToWorld[3][1] = origin.y;
			cameraToWorld[3][2] = origin.z;
			cameraToWorld[3][3] = 1;

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
