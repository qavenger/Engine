#include "../Shared/pch.h"
#include "CameraController.h"
#include "ComponentPool.h"
#include "../Input/Input.h"
#include "Entity.h"
#include "Transform.h"
#include "../GameApp/MathDefine.h"
//#include <fstream>
namespace EntityComponent
{
	DEFINE_COMPONENT_TYPE_ID(CameraController);
	void CameraController::Tick()
	{
		float speed = 3 * g_Timer.DeltaTime();
		if (Input::IsKeyHold(Input::KeyCode::LMouse))
		{
			int x, y;
			Input::GetMouseRelativePosition(x, y);
			Pitch(y * G_DEG2RAD);
			Yaw(x * G_DEG2RAD);
		}
		/*if (Input::IsKeyPressed(Input::KeyCode::T))
		{
			std::ofstream fout(L"Coord.txt");
			if (fout)
			{
				float3 pos;
				XMStoreFloat3(&pos, m_gameObject->GetTransform()->GetWorldPosition());
				float4 rot;
				XMStoreFloat4(&rot, m_gameObject->GetTransform()->GetWorldRotation());
				fout << pos.x << ", " << pos.y << ", " << pos.z << '\n';
				fout << rot.x << ", " << rot.y << ", " << rot.z << ", " << rot.w;
				fout.close();
			}
		}*/
		if (Input::IsKeyHold(Input::KeyCode::Shift))
		{
			speed *= 3;
		}
		if (Input::IsKeyHold(Input::KeyCode::W))
		{
			Forward(speed);
		}
		if (Input::IsKeyHold(Input::KeyCode::S))
		{
			Forward(-speed);
		}
		if (Input::IsKeyHold(Input::KeyCode::A))
		{
			Strafe(-speed);
		}
		if (Input::IsKeyHold(Input::KeyCode::D))
		{
			Strafe(speed);
		}
		if (Input::IsKeyHold(Input::KeyCode::E) || Input::IsKeyHold(Input::KeyCode::Space))
		{
			Fly(speed);
		}
		if (Input::IsKeyHold(Input::KeyCode::Q) || Input::IsKeyHold(Input::KeyCode::Control))
		{
			Fly(-speed);
		}
	}

	void CameraController::Rotate(float pitch, float yaw, float roll)
	{
		GetEntity()->GetTransform()->RotateLocal(pitch, yaw, roll);
	}

	void CameraController::Forward(float speed)
	{
		XMVECTOR velocity = XMVectorSet(0, 0, speed, 0);
		Transform* transform = GetEntity()->GetTransform();
		XMVECTOR dir = transform->GetWorldRotation();
		velocity = XMVector3Rotate(velocity, dir);
		transform->Translate(velocity);
	}

	void CameraController::Strafe(float speed)
	{
		XMVECTOR velocity = XMVectorSet(speed, 0, 0, 0);
		Transform* transform = GetEntity()->GetTransform();
		XMVECTOR dir = transform->GetWorldRotation();
		velocity = XMVector3Rotate(velocity, dir);
		transform->Translate(velocity);
	}

	void CameraController::Fly(float speed)
	{
		XMVECTOR velocity = XMVectorSet(0, speed, 0, 0);
		GetEntity()->GetTransform()->Translate(velocity);
	}

	void CameraController::Yaw(float speed)
	{
		GetEntity()->GetTransform()->RotateLocal(0, speed, 0);
	}

	void CameraController::Pitch(float speed)
	{
		GetEntity()->GetTransform()->Rotate(speed, 0, 0);
	}
}