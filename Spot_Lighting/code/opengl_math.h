#ifndef __OPENGL_MATH_H
#define __OPENGL_MATH_H

#include <math.h>
#include <string.h>

#define PI (3.14159265358979323846)
#define PI_DIV_180 (0.017453292519943296)
#define INV_PI_DIV_180 (57.2957795130823229)

#define DegToRad(x)  ((x)*PI_DIV_180)
#define RadToDeg(x)  ((x)*INV_PI_DIV_180)

//向量
typedef int     Vector2i[2];
typedef float   Vector2f[2];         
typedef float   Vector3f[3];                
typedef float   Vector4f[4];      
//向量赋值
inline void LoadVector2(Vector2f v, const float x, const float y)
{
	v[0] = x; v[1] = y;
}
inline void LoadVector3(Vector3f v, const float x, const float y, const float z)
{
	v[0] = x; v[1] = y; v[2] = z;
}
inline void LoadVector4(Vector4f v, const float x, const float y, const float z, const float w)
{
	v[0] = x; v[1] = y; v[2] = z; v[3] = w;
}
inline void SubVector3(Vector3f result,Vector3f l, Vector3f r)
{
	result[0] = l[0] - r[0];
	result[1] = l[1] - r[1];
	result[2] = l[2] - r[2];
}
inline void AddVector3(Vector3f result, Vector3f l, Vector3f r)
{
	result[0] = l[0] + r[0];
	result[1] = l[1] + r[1];
	result[2] = l[2] + r[2];
}
//向量复制
inline void CopyVector2(Vector2f dst, const Vector2f src) { memcpy(dst, src, sizeof(Vector2f)); }
inline void CopyVector3(Vector3f dst, const Vector3f src) { memcpy(dst, src, sizeof(Vector3f)); }
// 得到向量长度的平方
inline float GetVectorLengthSquared3(const Vector3f u)
{
	return (u[0] * u[0]) + (u[1] * u[1]) + (u[2] * u[2]);
}
// 得到向量长度
inline float GetVectorLength3(const Vector3f u)
{
	return sqrtf(GetVectorLengthSquared3(u));
}
//缩放向量
inline void ScaleVector3(Vector3f v, const float scale)
{
	v[0] *= scale; v[1] *= scale; v[2] *= scale;
}
//向量单位化
inline void NormalizeVector3(Vector3f u)
{
	ScaleVector3(u, 1.0f / GetVectorLength3(u));
}
//叉积
inline void CrossProduct3(Vector3f result, const Vector3f u, const Vector3f v)
{
	result[0] = u[1] * v[2] - v[1] * u[2];
	result[1] = u[2] * v[0] - v[2] * u[0];
	result[2] = u[0] * v[1] - v[0] * u[1];
}
inline void MultiplyVector4and4(Vector4f final, Vector4f l, Vector4f r)//四元数乘三维向量
{
	final[3] = (l[3] * r[3]) - (l[0] * r[0]) - (l[1] * r[1]) - (l[2] * r[2]);
	final[0] = (l[0] * r[3]) + (l[3] * r[0]) + (l[1] * r[2]) - (l[2] * r[1]);
	final[1] = (l[1] * r[3]) + (l[3] * r[1]) + (l[2] * r[0]) - (l[0] * r[2]);
	final[2] = (l[2] * r[3]) + (l[3] * r[2]) + (l[0] * r[1]) - (l[1] * r[0]);
}
inline void MultiplyVector4and3(Vector4f final, Vector4f q, Vector3f v)
{
	final[3] = -(q[0] * v[0]) - (q[1] * v[1]) - (q[2] * v[2]);
	final[0] = (q[3] * v[0]) + (q[1] * v[2]) - (q[2] * v[1]);
	final[1] = (q[3] * v[1]) + (q[2] * v[0]) - (q[0] * v[2]);
	final[2] = (q[3] * v[2]) + (q[0] * v[1]) - (q[1] * v[0]);
}
//向量v旋转a度：
//				W = Q * v * Q^(-1)
//	Q = ( Vx * sin(a/2), Vy * sin(a/2), Vz * sin(a/2), cos(a/2) )
//	Q^(-1) = ( -Vx * sin(a/2), -Vy * sin(a/2), -Vz * sin(a/2), cos(a/2) )
//v:要旋转的向量，Angle：旋转角度，Axe：围绕的向量
inline void RotateVector3(Vector3f v,float Angle,Vector3f Axe)
{
	const float SinHalfAngle = sinf(DegToRad(Angle / 2));
	const float CosHalfAngle = cosf(DegToRad(Angle / 2));

	const float Rx = Axe[0] * SinHalfAngle;
	const float Ry = Axe[1]* SinHalfAngle;
	const float Rz = Axe[2] * SinHalfAngle;
	const float Rw = CosHalfAngle;
	
	Vector4f RotationQ;//四元数
	LoadVector4(RotationQ,Rx, Ry, Rz, Rw);
	Vector4f ConjugateQ;
	LoadVector4(ConjugateQ, -Rx, -Ry, -Rz, Rw);
	//  ConjugateQ.Normalize();
	Vector4f W,V;
	MultiplyVector4and3(W, RotationQ, v); 
	MultiplyVector4and4(V, W, ConjugateQ);
	v[0] = V[0];
	v[1] = V[1]; 
	v[2] = V[2];
}


        
//	3 * 3 矩阵：
//   0       3       6
//   1       4       7
//   2       5       8
typedef float   Matrix33f[9];         


//  4 * 4 矩阵：
//      0       4       8       12
//      1       5       9       13
//      2       6       10      14
//      3       7       11      15
typedef float Matrix44f[16];         
//4*4单位矩阵
inline void LoadIdentity44(Matrix44f m)
{
	m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f;  m[12] = 0.0f;
	m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f;  m[13] = 0.0f;
	m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
	m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}
//4*4矩阵相乘
inline void MatrixMultiply44(Matrix44f product, const Matrix44f a, const Matrix44f b)
{
	unsigned int j, k;
	for (unsigned int i = 0; i < 16; i++) {
		j = i % 4;
		k = i / 4 * 4;
		product[i] = a[j] * b[k] + a[j + 4] * b[k + 1] + a[j + 8] * b[k + 2] + a[j + 12] * b[k + 3];
	}
}
//缩放变换
inline void ScaleMatrix44(Matrix44f m, float xScale, float yScale, float zScale)
{
	LoadIdentity44(m); m[0] = xScale; m[5] = yScale; m[10] = zScale;
}
//旋转变换
inline void RotationMatrix44(Matrix44f m, float angle, float x, float y, float z)
{
	LoadIdentity44(m);
	if (z == 1)//绕z轴
	{
		m[0] = cosf(angle); m[4] = -sinf(angle);
		m[1] = sinf(angle); m[5] = cosf(angle);
	}
	else if (y == 1)//绕y轴
	{
		m[0] = cosf(angle); m[8] = -sinf(angle);
		m[2] = sinf(angle); m[10] = cosf(angle);
	}
	else if (x == 1)//绕x轴
	{
		m[5] = cosf(angle); m[9] = -sinf(angle);
		m[6] = sinf(angle); m[10] = cosf(angle);
	}
}
inline void RotationMatrix44(Matrix44f m, float RotateX, float RotateY, float RotateZ)
{
	Matrix44f rx, ry, rz, temp;

	const float x = DegToRad(RotateX);
	const float y = DegToRad(RotateY);
	const float z = DegToRad(RotateZ);

	RotationMatrix44(rx, x, 1, 0, 0);
	RotationMatrix44(ry, y, 0, 1, 0);
	RotationMatrix44(rz, z, 0, 0, 1);

	MatrixMultiply44(temp, rz, ry);
	MatrixMultiply44(m,temp, rx);

}
//平移变换
inline void TranslationMatrix44(Matrix44f m, float x, float y, float z)
{
	LoadIdentity44(m); m[12] = x; m[13] = y; m[14] = z;
}
//透视投影配置参数
struct PersProjInfo
{
	float FOV;
	float Width;
	float Height;
	float zNear;
	float zFar;
};

//透视投影变换
/*
	 _														   _		
	|	a/aspect		0			0				0			|
	|	  0				a			0				0			|
	|	  0				0		-(f+n)/(f-n)	(2*f*n)/(f-n)	|
	|	  0				0			1				0			|
	|_														   _|

	a: 相当于焦距 大小为 1/tan(视野/2)  (ps:视野是一个角度)
	aspect:屏幕的宽高比 宽度/高度
	f: 到远处平面的距离
	n：到近处平面的距离
*/
inline void PersProjectionMatrix44(Matrix44f m, PersProjInfo p)
{
	const float ar = p.Width / p.Height;
	const float zRange = p.zNear - p.zFar;
	const float tanHalfFOV = tanf(DegToRad(p.FOV / 2.0f));

	m[0] = 1.0f / (tanHalfFOV * ar); m[4] = 0.0f;				 m[8] = 0.0f;						    m[12] = 0.0;
	m[1] = 0.0f;					 m[5] = 1.0f / tanHalfFOV;   m[9] = 0.0f;							m[13] = 0.0;
	m[2] = 0.0f;					 m[6] = 0.0f;			     m[10] = (-p.zNear - p.zFar) / zRange;  m[14] = 2.0f*p.zFar*p.zNear / zRange;
	m[3] = 0.0f;					 m[7] = 0.0f;				 m[11] = 1.0f;							m[15] = 0.0;

}

//UVN矩阵 
//相机转化矩阵
//N：相机目标朝向的方向向量(对应X轴)  V: 竖直站立时头顶到天空的方向(对应Y轴) U： 相机的右侧和x轴对应
inline void CameraMatrix44(Matrix44f m,Vector3f Target, Vector3f Up)//Target---N  Up----V
{
	Vector3f N,U,V,temp;
	for (unsigned int i = 0; i < 3; i++) {
		N[i] = Target[i];
		temp[i] = Up[i];
	}
	NormalizeVector3(N);
	CrossProduct3(U, temp, N); //U= Up * N
	NormalizeVector3(U);
	CrossProduct3(V, N, U); //V= N * U

	m[0] = U[0];   m[4] = U[1];   m[8] = U[2];    m[12] = 0.0f;
	m[1] = V[0];   m[5] = V[1];   m[9] = V[2];    m[13] = 0.0f;
	m[2] = N[0];   m[6] = N[1];   m[10] = N[2];   m[14] = 0.0f;
	m[3] = 0.0f;   m[7] = 0.0f;   m[11] = 0.0f;   m[15] = 1.0f;
}
#endif