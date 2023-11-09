#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
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
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		bool canRotate{ true };			// To enable/disable rotations when LMB + RMB is pressed/notpressed
		float movementSpeed{ 0.1f };
		float rotationSpeed{ 0.03f };
		float movementInc{ 4.f };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};			// Is just ONB ( Camera to World Space )
		Matrix viewMatrix{};			// World space to Camera space ( Inverse of the ONB )

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix	
			// ... First calculate the ONB
			
			// ... We calculate the Right & up vector using the forward camera vector
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			// Combine into a matrix including the origin
			// RIGHT
			invViewMatrix[0][0] = right.x;
			invViewMatrix[0][1] = right.y;
			invViewMatrix[0][2] = right.z;

			// UP
			invViewMatrix[1][0] = up.x;
			invViewMatrix[1][1] = up.y;
			invViewMatrix[1][2] = up.z;

			// Forward
			invViewMatrix[2][0] = forward.x;
			invViewMatrix[2][1] = forward.y;
			invViewMatrix[2][2] = forward.z;

			// Set camera origin as the translation
			invViewMatrix[3][0] = origin.x;
			invViewMatrix[3][1] = origin.y;
			invViewMatrix[3][2] = origin.z;
			invViewMatrix[3][3] = 1;

			//Inverse(ONB) => ViewMatrix
			// .. Inverse the ONB Matrix to get our ViewMatrix
			viewMatrix = invViewMatrix.Inverse();	

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W3

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			//...
			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			canRotate = true;
			UpdateMovement(pKeyboardState, mouseX, mouseY);

			if (canRotate)
				UpdateRotation(mouseX, mouseY);

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

		inline void UpdateMovement(const uint8_t* pKeyboardState, const int mouseX, const int mouseY)
		{
			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				movementInc = 4.f;
			}
			else
			{
				movementInc = 1.f;
			}

			// FORWARD / BACKWARD MOVEMENT
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * movementSpeed * movementInc;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * movementSpeed * movementInc;
			}

			// Forward / BackWard / Up / Down with mouse
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				// Left mouse button (LMB) is pressed
				if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))
				{
					// Right mouse button is also pressed
					origin += (up * static_cast<float>(-mouseY)) * movementSpeed * movementInc;
					canRotate = false;
				}
				else
				{
					// Only LMB is pressed
					origin += (forward * static_cast<float>(-mouseY)) * movementSpeed * movementInc;
				}
			}

			// LEFT / RIGHT MOVEMENT 
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * movementSpeed * movementInc;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * movementSpeed * movementInc;
			}
		}

		inline void UpdateRotation(int mouseX, int mouseY)
		{
			// Calculate total X rotation
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				// Left mouse button pressed
				totalPitch += static_cast<float>(mouseX) * rotationSpeed;
			}

			// Calculate total X/Y rotation
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT) &&
				!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
			{
				// Right mouse button pressed (And left one is not)
				totalPitch += static_cast<float>(mouseX) * rotationSpeed;
				totalYaw -= static_cast<float>(mouseY) * rotationSpeed;
			}


			Matrix finalRotation{ Matrix::CreateRotation(Vector3{ totalYaw, totalPitch, 0.f }) };

			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

		}

	};
}
