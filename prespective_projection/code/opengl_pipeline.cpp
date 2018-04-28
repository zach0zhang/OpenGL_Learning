#include "opengl_pipeline.h"
#include <stdio.h>
#include <Windows.h>

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

const Matrix44f * Pipeline::GetWPTrans()
{
	Matrix44f PersProjTrams;

	GetWorldTrans();

	PersProjectionMatrix44(PersProjTrams, m_persProjInfo);

	MatrixMultiply44(m_WPtransformation, PersProjTrams, m_Wtransformation);
	//m_WPtransformation = PersProjTrams * TranslationTrans * RotateTrans * ScaleTrans //先缩放，然后旋转，平移，最后投影

	return &m_WPtransformation;
}
