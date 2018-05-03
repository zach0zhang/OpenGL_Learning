#ifndef __OPENGL_CAMERA_H
#define __OPENGL_CAMERA_H
#include "opengl_math.h"
#include <gl/freeglut_std.h>

const static float STEP_SCALE = 1.0f;
const static float EDGE_STEP = 0.5f;
const static int MARGIN = 10;

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
	void Init()
	{
		Vector3f HTarget;
		LoadVector3(HTarget, m_target[0], 0.0, m_target[2]);
		NormalizeVector3(HTarget);

		if (HTarget[2] >= 0.0f)
		{
			if (HTarget[0] >= 0.0f)
			{
				m_AngleH = 360.0f - RadToDeg(asin(HTarget[2]));
			}
			else
			{
				m_AngleH = 180.0f + RadToDeg(asin(HTarget[2]));
			}
		}
		else
		{
			if (HTarget[0] >= 0.0f)
			{
				m_AngleH= RadToDeg(asin(-HTarget[2]));
			}
			else
			{
				m_AngleH=90.0f+ RadToDeg(asin(-HTarget[2]));
			}
		}

		m_AngleV = -RadToDeg(asin(m_target[1]));

		m_OnUpperEdge = false;
		m_OnLowerEdge = false;
		m_OnLeftEdge = false;
		m_OnRightEdge = false;
		m_mousePos[0] = m_windowWidth / 2;
		m_mousePos[1] = m_windowHeight / 2;

		glutWarpPointer(m_mousePos[0], m_mousePos[1]);

	}
	bool OnKeyboard(int key)
	{
		bool Ret = false;

		switch (key) {
		case GLUT_KEY_UP:
		{
			for (int i = 0; i < 3; i++)
				m_pos[i] += m_target[i];
			Ret = TRUE;
		}
		break;
		case GLUT_KEY_DOWN:
		{
			for (int i = 0; i < 3; i++)
				m_pos[i] -= m_target[i];
			Ret = TRUE;
		}
		break;
		case GLUT_KEY_LEFT:
		{
			Vector3f Left;
			CrossProduct3(Left, m_up, m_target);
			NormalizeVector3(Left);
			for (int i = 0; i < 3; i++)
				m_pos[i] += Left[i];
			Ret = TRUE;
		}
		break;
		case GLUT_KEY_RIGHT:
		{
			Vector3f Right;
			CrossProduct3(Right,m_target,m_up);
			NormalizeVector3(Right);
			for (int i = 0; i < 3; i++)
				m_pos[i] += Right[i];
			Ret = TRUE;
		}
		break;
		}
		return Ret;
	}
	void OnMouse(int x, int y)
	{
		const int DeltaX = x - m_mousePos[0];
		const int DeltaY = y - m_mousePos[1];

		m_mousePos[0] = x;
		m_mousePos[1] = y;

		m_AngleH += (float)DeltaX / 20.0f;
		m_AngleV += (float)DeltaY / 20.0f;

		if (DeltaX == 0) {
			if (x <= MARGIN) {
				//    m_AngleH -= 1.0f;
				m_OnLeftEdge = true;
			}
			else if (x >= (m_windowWidth - MARGIN)) {
				//    m_AngleH += 1.0f;
				m_OnRightEdge = true;
			}
		}
		else {
			m_OnLeftEdge = false;
			m_OnRightEdge = false;
		}
		if (DeltaY == 0) {
			if (y <= MARGIN) {
				m_OnUpperEdge = true;
			}
			else if (y >= (m_windowHeight - MARGIN)) {
				m_OnLowerEdge = true;
			}
		}
		else {
			m_OnUpperEdge = false;
			m_OnLowerEdge = false;
		}
		Update();
	}
	void Update()
	{
		Vector3f Vaxis;
		LoadVector3(Vaxis,0.0f, 1.0f, 0.0f);
		// Rotate the view vector by the horizontal angle around the vertical axis
		Vector3f View;
		LoadVector3(View, 1.0f, 0.0f, 0.0f);
		RotateVector3(View, m_AngleH, Vaxis);
		NormalizeVector3(View);

		// Rotate the view vector by the vertical angle around the horizontal axis
		Vector3f Haxis;
		CrossProduct3(Haxis,Vaxis,View);
		NormalizeVector3(Haxis);
		
		RotateVector3(View, m_AngleV, Haxis);

		CopyVector3(m_target, View);
		NormalizeVector3(m_target);

		CrossProduct3(m_up, m_target, Haxis);
		NormalizeVector3(m_up);
	}
	void OnRender()
	{
		bool ShouldUpdate = false;

		if (m_OnLeftEdge) {
			m_AngleH -= EDGE_STEP;
			ShouldUpdate = true;
		}
		else if (m_OnRightEdge) {
			m_AngleH += EDGE_STEP;
			ShouldUpdate = true;
		}

		if (m_OnUpperEdge) {
			if (m_AngleV > -90.0f) {
				m_AngleV -= EDGE_STEP;
				ShouldUpdate = true;
			}
		}
		else if (m_OnLowerEdge) {
			if (m_AngleV < 90.0f) {
				m_AngleV += EDGE_STEP;
				ShouldUpdate = true;
			}
		}

		if (ShouldUpdate) {
			Update();
		}
	}
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