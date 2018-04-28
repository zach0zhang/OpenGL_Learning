#include "opengl_pipeline.h"


const Matrix44f * Pipeline::GetWorldTrans()
{
	Matrix44f ScaleTrans, RotateTrans, TranslationTrans, temp;

	ScaleMatrix44(ScaleTrans, m_scale[0], m_scale[1], m_scale[2]);
	RotationMatrix44(RotateTrans, m_rotateInfo[0], m_rotateInfo[1], m_rotateInfo[2]);
	TranslationMatrix44(TranslationTrans, m_worldPos[0], m_worldPos[1], m_worldPos[2]);

	MatrixMultiply44(temp, TranslationTrans, RotateTrans);//temp = TranslationTrans * RotateTrans
	MatrixMultiply44(m_Wtransformation, temp, ScaleTrans); //m_Wtransformation = temp * ScaleTrans
	//m_Wtransformation=TranslationTrans * RotateTrans * ScaleTrans //先缩放，再旋转 最后平移
	return &m_Wtransformation;
}

const Matrix44f * Pipeline::GetProjTrans()
{
	PersProjectionMatrix44(m_ProjTransformation, m_persProjInfo);
	return &m_ProjTransformation;
}

const Matrix44f * Pipeline::GetViewTrans()
{
	Matrix44f CameraTranslationTrans, CameraRotateTrans;
	TranslationMatrix44(CameraTranslationTrans, -m_camera.Pos[0], -m_camera.Pos[1], -m_camera.Pos[2]);
	CameraMatrix44(CameraRotateTrans, m_camera.Target, m_camera.Up);

	MatrixMultiply44(m_Vtransformation, CameraRotateTrans, CameraTranslationTrans);
	return &m_Vtransformation;
}

const Matrix44f * Pipeline::GetWPTrans()
{
	Matrix44f PersProjTrams;

	GetWorldTrans();
	PersProjectionMatrix44(PersProjTrams, m_persProjInfo);
	MatrixMultiply44(m_WPtransformation, PersProjTrams, m_Wtransformation);
	//m_WPtransformation = PersProjTrams * TranslationTrans * RotateTrans * ScaleTrans //先缩放，然后旋转，平移，最后投影
	return &m_WPtransformation;
}

const Matrix44f * Pipeline::GetVPTrans()
{
	GetViewTrans();
	GetProjTrans();

	MatrixMultiply44(m_VPtransformation, m_ProjTransformation, m_Vtransformation);
	return &m_VPtransformation;
}

const Matrix44f * Pipeline::GetWVPTrans()
{
	GetWorldTrans();
	GetVPTrans();

	MatrixMultiply44(m_WVPtransformation, m_VPtransformation, m_Wtransformation);
	return &m_WVPtransformation;;
}
