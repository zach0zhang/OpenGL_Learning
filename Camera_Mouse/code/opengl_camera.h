#ifndef __OPENGL_CAMERA_H
#define __OPENGL_CAMERA_H
#include "opengl_math.h"
#include <gl/freeglut_std.h>

class Camera
{
private:
	Vector3f m_pos;
	Vector3f m_target;
	Vector3f m_up;

	float m_AngleH;
	float m_AngleV;

	bool m_OnUpperEdge;
	bool m_OnLowerEdge;
	bool m_OnLeftEdge;
	bool m_OnRightEdge;

	Vector2i m_mousePos;

	int m_windowWidth;
	int m_windowHeight;
public:
	Camera(int Window_Width, int Window_Height)
	{
		m_windowWidth = Window_Width;
		m_windowHeight = Window_Height;
		LoadVector3(m_pos,0.0f, 0.0f, 0.0f);
		LoadVector3(m_target,0.0f, 0.0f, 1.0f);
		LoadVector3(m_target,0.0f, 1.0f, 0.0f);
	}
	Camera(int Window_Width, int Window_Height, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
	{
		m_windowWidth = Window_Width;
		m_windowHeight = Window_Height;
		CopyVector3(m_pos, Pos);
		CopyVector3(m_target, Target);
		NormalizeVector3(m_target);
		CopyVector3(m_up, Up);
		NormalizeVector3(m_up);

		Init();
	}
	void Init();
	bool OnKeyboard(int key);
	void OnMouse(int x, int y);
	void Update();
	void OnRender();
	const Vector3f& GetPos()
	{
		return m_pos;
	}
	const Vector3f& GetTarget()
	{
		return m_target;
	}
	const Vector3f& GetUp()
	{
		return m_up;
	}
};

#endif