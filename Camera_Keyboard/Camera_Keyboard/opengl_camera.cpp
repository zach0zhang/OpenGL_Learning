#include "opengl_camera.h"
#include <gl/freeglut_std.h>
#include <stdio.h>
#define StepSize 1
bool Camera::OnKeyboard(int key)
{
	bool Ret = false;

	switch (key) {
	case GLUT_KEY_UP:
	{
		for (int i = 0; i < 3; i++)
			m_pos[i] += m_target[i] * StepSize;
		Ret = TRUE;
	}
	break;
	case GLUT_KEY_DOWN:
	{
		for (int i = 0; i < 3; i++)
			m_pos[i] -= m_target[i] * StepSize;
		Ret = TRUE;
	}
	break;
	case GLUT_KEY_LEFT:
	{
		Vector3f Left;
		CrossProduct3(Left, m_target, m_up);
		NormalizeVector3(Left);
		for (int i = 0; i < 3; i++)
			m_pos[i] += Left[i] * StepSize;
		Ret = TRUE;
	}
	break;
	case GLUT_KEY_RIGHT:
	{
		Vector3f Right;
		CrossProduct3(Right, m_up,m_target);
		NormalizeVector3(Right);
		for (int i = 0; i < 3; i++)
			m_pos[i] += Right[i] * StepSize;
		Ret = TRUE;
	}
	break;
	}
	return Ret;
}