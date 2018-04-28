根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

## 一、混合变换
之前的教程中，学习了如何让物体平移、旋转和缩放，相关教程链接：

[OpenGL学习之路6----平移，旋转和缩放变换](https://blog.csdn.net/zach_z/article/details/80072929)

### 1.1 一次性计算变换矩阵
我们每进行一次变换都要左乘一次我们的变换矩阵，如果每进行一次变换(平移、旋转或者缩放)都在着色器里左乘一次变换矩阵，则会显得非常低效和臃肿。

线性代数中对于给定的一组矩阵M0,M1,M2...Mn和一个向量V具有下列等式

```math
M_n * M_{n-1} * ... * M_0 * V = (M_n* M_{n-1} * ... * M_0) * V
```
我们可以一次性计算(Mn * Mn-1 * ... * M0),然后把计算结果作为Uniform变量传递给着色器，让着色器跟每个顶点位置相乘完成变换。

### 1.2 变换矩阵的先后顺序

首先，我们要明白：**向量最开始先被最右边的矩阵左乘，即按照最右矩阵到最左矩阵顺序进行变换**

一个正方形先旋转再平移如图：
![image](E:/笔记/Opengl/Concatenating_Transformations/1.png)
先平移再旋转如图：
![image](E:/笔记/Opengl/Concatenating_Transformations/2.png)
通过观察两种不同顺序的变换，我们不难发现，先平移会使物体远离坐标原点导致物体旋转绕原点旋转而不是绕自身旋转(旋转也会造成平移效果)，而先旋转或者缩放物体不会使物体出现这种副作用。

所以我们对物体进行混合变换的顺序是 **先缩放  再旋转 最后再平移**

## 二、自定义一个管线类
本节中我们自定义一个管线类，用来抽象出一个物体所有变换的数据信息。

### 2.1 opengl_pipeline.h:

```
#ifndef __OPENGL_PIPELINE_H
#define __OPENGL_PIPELINE_H

#include "opengl_math.h"

class Pipeline
{
private:
	Vector3f m_scale;
	Vector3f m_worldPos;
	Vector3f m_rotateInfo;
	Matrix44f m_transformation;
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

	const Matrix44f* GetTrans();

};

#endif 
```
- 根据代码可以看到自定义的管线类定义了**三个三维向量变量来存储物体在世界空间中的缩放比例，位置和旋转角度**
- 并且有构造方法在没有设置三个变化变量时，默认不缩放(缩放比例为1)，不平移，不旋转
- 三个接口函数可以设置三个变化变量的值
- 一个私有4*4的矩阵变量，存储经过混合变换后变换矩阵的值
- 一个GetTrans()方法用来计算混合变换矩阵的值(在opengl_pipeline.c中实现)

### 2.2 opengl_pipeline.cpp:

```
#include "opengl_pipeline.h"


const Matrix44f * Pipeline::GetTrans()
{
	Matrix44f ScaleTrans, RotateTrans, TranslationTrans, temp;

	ScaleMatrix44(ScaleTrans, m_scale[0], m_scale[1], m_scale[2]);
	RotationMatrix44(RotateTrans, m_rotateInfo[0], m_rotateInfo[1], m_rotateInfo[2]);
	TranslationMatrix44(TranslationTrans, m_worldPos[0], m_worldPos[1], m_worldPos[2]);

	MatrixMultiply44(temp, TranslationTrans, RotateTrans);//temp = TranslationTrans * RotateTrans
	MatrixMultiply44(m_transformation, temp, ScaleTrans); //m_transformation = temp * ScaleTrans
	return &m_transformation;
}

```
- 根据代码可以看到分别根据管线类中的m_scale(缩放变量)，m_rotateInfo(旋转变量)和m_worldPos(位置变量)，调用3d函数库中的内联函数来计算相应的ScaleTrans(缩放矩阵)，RotateTrans(旋转矩阵)和TranslationTrans(平移矩阵)
- 再按照TranslationTrans * RotateTrans * ScaleTrans的顺序(即先缩放再旋转最后平移)计算出最后的混合变换矩阵

## 三、使用管线类并且得到混合变化矩阵

```
    Pipeline p;
	p.Scale(sinf(Scale*0.1f), sinf(Scale*0.1f), sinf(Scale*0.1f));
	p.WorldPos(sinf(Scale), 0.0f, 0.0f);
	p.Rotate(sinf(Scale)*90.0f, sinf(Scale)*90.0f, sinf(Scale)*90.0f);
	glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, (const GLfloat*)p.GetTrans());
```
- 先实例化了一个Pipeine,P,此时P的三个变换变量根据Pipeline类的构造方法初始化
- 使用三个接口函数分别跟三个变换变量赋值
- 调用GetTrans()获得混合变化矩阵，并作为Uniform变量传递到着色器

## 四、其他代码
其他代码相比于上一节并无本质上变化，包括主程序、3d数学库和着色器
opengl_math.h:

```
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
typedef float   Vector3f[3];

//向量赋值

inline void LoadVector3(Vector3f v, const float x, const float y, const float z)
{
	v[0] = x; v[1] = y; v[2] = z;
}
//缩放向量
inline void ScaleVector3(Vector3f v, const float scale)
{
	v[0] *= scale; v[1] *= scale; v[2] *= scale;
}
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
	MatrixMultiply44(m, temp, rx);

}
//平移变换
inline void TranslationMatrix44(Matrix44f m, float x, float y, float z)
{
	LoadIdentity44(m); m[12] = x; m[13] = y; m[14] = z;
}
#endif
```
main.cpp:

```
#include <stdio.h>
#include <string>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <assert.h>

#include "opengl_math.h"
#include "opengl_pipeline.h"
using namespace std;

GLuint VBO;
GLuint gWorldLocation;
GLuint IBO;


const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

bool ReadFile(const char* pFileName, std::string &outFile)
{
	std::ifstream f(pFileName);
	bool ret = FALSE;
	
	if (f.is_open()) {
		std::string line;
		while (getline(f, line)) {
			outFile.append(line);
			outFile.append("\n");
		}
		f.close();
		ret = TRUE;
	}
	else {
		fprintf(stderr, "%s:%d: unable to open file '%s'\n", __FILE__, __LINE__, pFileName);
		system("pause");
	}
	return ret;
}

static void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;

	Scale += 0.001f;

	//实例化一个Pipeline 
	Pipeline p;
	p.Scale(sinf(Scale*0.1f), sinf(Scale*0.1f), sinf(Scale*0.1f));
	p.WorldPos(sinf(Scale), 0.0f, 0.0f);
	p.Rotate(sinf(Scale)*90.0f, sinf(Scale)*90.0f, sinf(Scale)*90.0f);
	glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, (const GLfloat*)p.GetTrans());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}

static void CreateVertexBuffer()
{
	Vector3f Vertices[4];
	LoadVector3(Vertices[0], -1.0f, -1.0f, 0.0f);
	LoadVector3(Vertices[1], 0.0f, -1.0f, 1.0f);
	LoadVector3(Vertices[2], 1.0f, -1.0f, 0.0f);
	LoadVector3(Vertices[3], 0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	GLuint Indices[] = {
		0,3,1,
		1,3,2,
		2,3,0,
		0,1,2
	};

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW); 
}

static void AddShader(GLuint ShaderProgram, const char* ShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj) {
		fprintf(stderr, "Error about create Shader object");
		system("pause");
		exit(1);
	}
	const GLchar* p[1];
	p[0] = ShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(ShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS,&success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		system("pause");
		exit(1);
	}
	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShader()
{
	GLuint ShaderProgram = glCreateProgram();
	if (!ShaderProgram) {
		fprintf(stderr, "Error about Create Shader Program\n");
		system("pause");
		exit(1);
	}

	std::string vs, fs;
	if (!ReadFile(pVSFileName,vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName,fs)) {
		exit(1);
	}

	AddShader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram,GL_LINK_STATUS,&Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog),NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n",ErrorLog);
		system("pause");
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		system("pause");
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(Render);

	glutIdleFunc(Render);
}

int main(int argv, char **argc)
{
	glutInit(&argv, argc);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(1024, 768);
	glutCreateWindow("Concatenating Transformation");

	InitializeGlutCallbacks();

	GLenum res = glewInit();
	if (res!=GLEW_OK) {
		fprintf(stderr, "Error:'%s'\n", glewGetErrorString(res));
		system("pause");
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();

	CompileShader();

	glutMainLoop();

	return 0;
}
```
shader.vs:

```
#version 330

layout (location = 0) in vec3 Position;

uniform mat4 gWorld;

out vec4 Color;

void main()
{
	gl_Position =  gWorld*vec4(Position, 1.0);
	Color = vec4(clamp(Position, 0.0,1.0),1.0);
}
```
shader.fs:

```
#version 330

in vec4 Color;

out vec4 FragColor;

void main()
{
	FragColor=Color;
}
```

## 五、运行结果
![image](E:/笔记/Opengl/Concatenating_Transformations/3.png)
可以看到四面体在空间中大大小小飞来飞去，具体效果可以在渲染回调函数Render()里的Pipeline设置三个变换变量那里设置。

