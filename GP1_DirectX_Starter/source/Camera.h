#ifndef DirectX_CAMERA
#define DirectX_CAMERA

namespace dae
{
	class Camera final
	{
	public:
		Camera() = default;
		Camera(const Vector3& origin, float fovAngle );

		void Initialize(float aspectRatio, float fovAngle = 90.f, Vector3 origin = { 0.f,0.f,0.f });

		void Update(const Timer* pTimer);

		Matrix GetViewMatrix() const;
		Matrix GetProjectionMatrix() const;

	private:
		Vector3 m_Origin;
		float m_FovAngle;
		float m_Fov;
		float m_AspectRatio;
		float m_Near_plane;
		float m_Far_plane;

		Vector3 m_Forward;
		Vector3 m_Up;
		Vector3 m_Right;

		bool m_CanRotate;						// To enable/disable rotations when LMB + RMB is pressed/notpressed
		float m_MovementSpeed;
		float m_RotationSpeed;
		float m_MovementInc;

		float m_TotalPitch;
		float m_TotalYaw;


		Matrix m_InvViewMatrix;					// Is just ONB ( Camera to World Space )
		Matrix m_ViewMatrix;					// World space to Camera space ( Inverse of the ONB )
		Matrix m_ProjectionMatrix;

		inline void CalculateViewMatrix();
		inline void CalculateProjectionMatrix();

		inline void UpdateMovement(float deltaTime, const uint8_t* pKeyboardState, const int mouseX, const int mouseY);
		inline void UpdateRotation(float deltaTime, int mouseX, int mouseY);
	};
}


#endif

