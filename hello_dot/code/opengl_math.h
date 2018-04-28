#ifndef __OPENGL_MATH_H
#define __OPENGL_MATH_H

//向量        
typedef float   Vector3f[3];                

//向量赋值
inline void LoadVector3(Vector3f v, const float x, const float y, const float z)
{
	v[0] = x; v[1] = y; v[2] = z;
}

#endif