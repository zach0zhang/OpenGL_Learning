#ifndef __OPENGL_PIPELINE_H
#define __OPENGL_PIPELINE_H

#include "opengl_math.h"
#include "opengl_camera.h"

class Pipeline
{
private:
	Vector3f m_scale;
	Vector3f m_worldPos;
	Vector3f m_rotateInfo;

	Matrix44f m_Wtransformation;
	Matrix44f m_ProjTransformation;
	Matrix44f m_Vtransformation;
	Matrix44f m_WPtransformation;
	Matrix44f m_VPtransformation;
	Matrix44f m_WVPtransformation;

	PersProjInfo m_persProjInfo;

	struct {
		Vector3f Pos;
		Vector3f Target;
		Vector3f Up;
	} m_camera;

public:
	Pipeline() {
		LoadVector3(m_scale,1.0f, 1.0f, 1.0f);
		LoadVector3(m_worldPos,0.0f, 0.0f, 0.0f);
		LoadVector3(m_rotateInfo,0.0f, 0.0f, 0.0f);
	}
	void Scale(float ScaleX, float ScaleY, float ScaleZ)
	{
		LoadVector3(m_scale, ScaleX, ScaleY, ScaleZ);
	}

	void WorldPos(float x, float y, float z)
	{
		LoadVector3(m_worldPos, x, y, z);
	}
	void Rotate(float RotateX, float RotateY, float RotateZ)
	{
		LoadVector3(m_rotateInfo, RotateX, RotateY, RotateZ);
	}
	void SetPerspectiveProj(const PersProjInfo& p)
	{
		m_persProjInfo = p;
	}
	void SetCamera(Vector3f Pos, Vector3f Target, Vector3f Up)
	{
		/*
		for (unsigned int i = 0; i < 3; i++)
		{
			m_camera.Pos[i] = Pos[i];
			m_camera.Target[i] = Target[i];
			m_camera.Up[i] = Up[i];
		}
		*/
		CopyVector3(m_camera.Pos, Pos);
		CopyVector3(m_camera.Target, Target);
		CopyVector3(m_camera.Up, Up);
	}
	void SetCamera(Camera& Camera)
	{
		CopyVector3(m_camera.Pos, Camera.GetPos());
		CopyVector3(m_camera.Target, Camera.GetTarget());
		CopyVector3(m_camera.Up, Camera.GetUp());
	}

	const Matrix44f* GetWorldTrans();
	const Matrix44f* GetProjTrans();
	const Matrix44f* GetViewTrans();
	const Matrix44f* GetWPTrans();
	const Matrix44f* GetVPTrans();
	const Matrix44f* GetWVPTrans();

};

#endif 