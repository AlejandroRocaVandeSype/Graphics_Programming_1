#include "pch.h"
#include "Camera.h"


using namespace dae;


Camera::Camera(const Vector3& origin, float fovAngle)
	: m_Origin{ origin }, m_FovAngle{ fovAngle }, m_Fov{},
	m_AspectRatio{}, m_Near_plane{ 0.1f }, m_Far_plane{ 100.f},
	m_Forward{ Vector3::UnitZ }, m_Up{ Vector3::UnitY }, m_Right{ Vector3::UnitX },
	m_CanRotate{ true }, m_MovementSpeed{ 9.f }, m_RotationSpeed{ 0.6f }, m_MovementInc{ 4.f },
	m_TotalPitch{}, m_TotalYaw{}, m_InvViewMatrix{}, m_ViewMatrix{}, m_ProjectionMatrix{}
{

}


void Camera::Initialize(float aspectRatio, float fovAngle, Vector3 origin)
{
	m_FovAngle = fovAngle;
	m_Fov = tanf((fovAngle * TO_RADIANS) / 2.f);
	m_AspectRatio = aspectRatio;
	m_Origin = origin;
	m_Near_plane = 0.1f;
	m_Far_plane = 100.f;
	m_Up = Vector3::UnitY;
	m_Right = Vector3::UnitX;
	m_Forward = Vector3::UnitZ;
	m_MovementInc = 4.f;
	m_RotationSpeed = 8.f;
	m_MovementSpeed = 12.f;
	m_CanRotate = true;
}


void Camera::Update(const Timer* pTimer)
{
	const float deltaTime = pTimer->GetElapsed();

	//Camera Update Logic
	//...
	//Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	//Mouse Input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	m_CanRotate = true;
	UpdateMovement(deltaTime, pKeyboardState, mouseX, mouseY);

	if (m_CanRotate)
		UpdateRotation(deltaTime, mouseX, mouseY);

	//Update Matrices
	CalculateViewMatrix();
	CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
}

inline void Camera::UpdateMovement(float deltaTime, const uint8_t* pKeyboardState, const int mouseX, const int mouseY)
{
	if (pKeyboardState[SDL_SCANCODE_LSHIFT])
	{
		m_MovementInc = 4.f;
	}
	else
	{
		m_MovementInc = 1.f;
	}

	// FORWARD / BACKWARD MOVEMENT
	if (pKeyboardState[SDL_SCANCODE_W])
	{
		m_Origin += m_Forward * m_MovementSpeed * m_MovementInc * deltaTime;


	}
	if (pKeyboardState[SDL_SCANCODE_S])
	{
		m_Origin -= m_Forward * m_MovementSpeed * m_MovementInc * deltaTime;

	}

	// Forward / BackWard / Up / Down with mouse
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		// Left mouse button (LMB) is pressed
		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			// Right mouse button is also pressed
			m_Origin += (m_Up * static_cast<float>(-mouseY)) * m_MovementSpeed * m_MovementInc * deltaTime;
			m_CanRotate = false;
		}
		else
		{
			// Only LMB is pressed
			m_Origin += (m_Forward * static_cast<float>(-mouseY)) * m_MovementSpeed * m_MovementInc * deltaTime;
		}
	}

	// LEFT / RIGHT MOVEMENT 
	if (pKeyboardState[SDL_SCANCODE_A])
	{
		m_Origin -= m_Right * m_MovementSpeed * m_MovementInc * deltaTime;
	}
	if (pKeyboardState[SDL_SCANCODE_D])
	{
		m_Origin += m_Right * m_MovementSpeed * m_MovementInc * deltaTime;
	}

}

inline void Camera::UpdateRotation(float deltaTime, int mouseX, int mouseY)
{
	// Calculate total X rotation
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		// Left mouse button pressed
		m_TotalPitch += static_cast<float>(mouseX) * m_RotationSpeed * deltaTime;
	}

	// Calculate total X/Y rotation
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT) &&
		!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
	{
		// Right mouse button pressed (And left one is not)
		m_TotalPitch += static_cast<float>(mouseX) * m_RotationSpeed * deltaTime;
		m_TotalYaw -= static_cast<float>(mouseY) * m_RotationSpeed * deltaTime;
	}


	Matrix finalRotation{ Matrix::CreateRotation(Vector3{ m_TotalYaw, m_TotalPitch, 0.f }) };

	m_Forward = finalRotation.TransformVector(Vector3::UnitZ);
	m_Forward.Normalize();
}


inline void Camera::CalculateViewMatrix()
{
	//ONB => m_ViewMatrix	
			// ... First calculate the ONB

			// ... We calculate the Right & up vector using the forward camera vector
	m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
	m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();

	// Combine into a matrix including the origin
	// RIGHT
	m_ViewMatrix[0][0] = m_Right.x;
	m_ViewMatrix[0][1] = m_Right.y;
	m_ViewMatrix[0][2] = m_Right.z;

	// UP
	m_ViewMatrix[1][0] = m_Up.x;
	m_ViewMatrix[1][1] = m_Up.y;
	m_ViewMatrix[1][2] = m_Up.z;

	// Forward
	m_ViewMatrix[2][0] = m_Forward.x;
	m_ViewMatrix[2][1] = m_Forward.y;
	m_ViewMatrix[2][2] = m_Forward.z;

	// Set camera origin as the translation
	m_ViewMatrix[3][0] = m_Origin.x;
	m_ViewMatrix[3][1] = m_Origin.y;
	m_ViewMatrix[3][2] = m_Origin.z;
	m_ViewMatrix[3][3] = 1;

	//Inverse(ONB) => ViewMatrix
	// .. Inverse the ONB Matrix to get our ViewMatrix
	m_ViewMatrix = m_ViewMatrix.Inverse();

	//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
	//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
}

inline void Camera::CalculateProjectionMatrix()
{
	m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_Fov, m_AspectRatio, m_Near_plane, m_Far_plane);
}


Matrix Camera::GetViewMatrix() const
{
	return m_ViewMatrix;
}
Matrix Camera::GetProjectionMatrix() const
{
	return m_ProjectionMatrix;
}