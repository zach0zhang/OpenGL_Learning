#ifndef __OPENGL_PIPELINE_H
#define __OPENGL_PIPELINE_H

#include "opengl_math.h"

class Pipeline
{
private:
	Vector3f m_scale;
	Vector3f m_worldPos;
	Vector3f m_rotateInfo;

	Matrix44f m_Wtransformation;
	Matrix44f m_ProjTransformation;
	Matrix44f m_WPtransformation;

	PersProjInfo m_persProjInfo;
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

	const Matrix44f* GetWorldTrans();
	const Matrix44f* GetProjTrans();
	const Matrix44f* GetWPTrans();

};

#endif 