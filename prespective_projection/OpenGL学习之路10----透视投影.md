根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

## 一、透视投影
### 1.1 透视投影定义
中心投影法：
- 光线照射物体时，可在预设的地面或墙面上产生影子，根据这一自然现象，经科学的抽象总结，产生了投影法。
- 投射线通过物体，向选定的面投射，并在该面上得到图形的方法称为投影法
- 投射线汇交于投射中心的投影法叫做中心投影法

中心投影法示意图：
![image](E:/笔记/Opengl/Perspective_Projection/1.png)

**透视投影是用中心投影法将形体投射到投影面上，从而获得的一种较为接近视觉效果的单面投影图**

透视投影符合人们心理习惯，即离视点近的物体大，离视点远的物体小，远到极点即为消失，成为灭点

透视投影示意图：
![image](E:/笔记/Opengl/Perspective_Projection/2.png)

### 1.2 视锥体
当使用投影矩阵，场景中的并行线会在屏幕上的一个消失点处汇聚到一起，物体离得越远，则变得越小。看到的空间区域被称为视椎体。

通过视椎体投影： 
![image](E:/笔记/Opengl/Perspective_Projection/3.png)

### 1.3 必要的参数
- **90°视野和45°视野：** 
![image](E:/笔记/Opengl/Perspective_Projection/5.png)

- **屏幕的宽高比**: 因为我们在一个宽高相等(-1到1)的单位化窗口里展示坐标系,而通常电脑屏幕的宽度是大于高度的(比如1024*768)，所以需要在水平方向上的轴线上步骤更加密集的坐标点，竖直方向上相对稀疏。
- **到远处和近处的距离**：需要把离相机太近和太远的物体裁掉

### 1.4 透视投影矩阵
正如我们之前教程对要绘制的图形进行平移、旋转和缩放操作时，则用平移矩阵、旋转矩阵和缩放矩阵跟图形的位置坐标进行相乘即可得到变换后的图形坐标。(可以点击链接进入教程学习)
[OpenGL学习之路6----平移，旋转和缩放变换](https://blog.csdn.net/zach_z/article/details/80072929)

我们对图形进行一个透视投影操作，也其实就是找到透视投影矩阵跟我们的图形位置相乘得到透视投影后的图形位置。

如果想了解投影矩阵的详细推导步骤请移步原教程
[Perspective Projection](http://ogldev.atspace.co.uk/www/tutorial12/tutorial12.html)

这里直接给出透视投影矩阵并稍作解释
![image](E:/笔记/Opengl/Perspective_Projection/4.png)
- a: 相当于焦距，大小为 1/tan(视野/2)
        
        即当视野为90°，则a=1/tan(90°/2)=1
- aspect：屏幕的宽高比，宽度/高度 
- f：到远处平面的距离，必须正值且大于到近处平面的距离 
- n：到近处平面的距离，必须是正值

### 1.5 代码中得到一个透视投影矩阵

```
struct PersProjInfo
{
	float FOV;
	float Width;
	float Height;
	float zNear;
	float zFar;
};
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
```
- 设置一个结构体用来存放透视投影的配置参数，分别是视野(角度)、屏幕宽度、屏幕高度、到近处平面的距离和到远处平面的距离
- 通过一个内联函数PersProjectionMatrix44()传入要设置的透视投影矩阵和配置参数结构体实例，设置相应参数的透视投影的值

## 二、代码解释
### 2.1 增加管线类的相关代码
opengl_pipeline.h:

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
```
- 通过对上一节的学习，我们的管线最后目的是获得一个最终想要的变换矩阵作为Uniform变量传递给着色器
- 这次给管线类增加了我们在opengl_math.h中定义的透视投影配置参数的结构体变量，用来配置管线内中的透视投影矩阵
- 管线类还增加了两个4*4的矩阵变量，分别代表透视投影矩阵，以及先进行混合变换再进行透视投影操作后的矩阵(即m_Wtransformation * m_ProjTransformation)
- 两个方法用来计算m_ProjTransformation矩阵和m_WPtransformation矩阵

opengl_pipeline.c:

```
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

```
可以看到在上一节获得混合变换矩阵的方法基础上，实现了获得透视投影矩阵的方法和混合变换再透视投影操作之后的变换矩阵的方法

### 2.2 主程序中的引用操作
main.cpp:

```
int main(int argc, char **argv)
{
    ...
	gPersProjInfo.FOV = 30.0f;
	gPersProjInfo.Height = 768;
	gPersProjInfo.Width = 1024;
	gPersProjInfo.zNear = 1.0f;
	gPersProjInfo.zFar = 100.0f;
    ...
}
```
- gPersProjInfo是一个PersProjInfo结构体的实例，是全局变量
- 主函数中对全局结构体变量gPersProjInfo的各个分量进行赋值
- 视野30°,屏幕宽为1024，高为768，到最近最远距离是1和100


```
Render()
{
    Pipeline p;
	p.Rotate(0.0f, Scale, 0.0f);
	p.WorldPos(0.0f, 0.0f, 5.0f);
	p.SetPerspectiveProj(gPersProjInfo);
	glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, (const GLfloat*)p.GetWPTrans());
}
```
- 把要绘制的图形延Z轴平移了五个单位(为了能观察到物体)，并且让物体绕y轴旋转一个角度
- 渲染回调函数Render()中使用管线类的SetPerspectiveProj()方法设置了管线类内部变量透视投影的参数
- 使用管线类的GetWPTrans()方法获得先进行混合变换(先缩放再旋转再平移)，这里只需要绕着y轴旋转即可，再进行透视投影操作后的矩阵
- 将最后得到的矩阵通过glUniformMatrix4fv()作为Uniform变量的值传到着色器


```
CreateVertexBuffer()
{
    ...
    LoadVector3(Vertices[0], -1.0f, -1.0f, 0.5773f);
	LoadVector3(Vertices[1], 0.0f, -1.0f, -1.15475f);
	LoadVector3(Vertices[2], 1.0f, -1.0f, 0.5773f);
	LoadVector3(Vertices[3], 0.0f, 1.0f, 0.0f);
	...
}
```
这是四面体的四个顶点,(因为渲染的时候会让物体向z轴正方向移动五个单位，所以能观察到这些点组成的四面体)通过透视投影可以看到立体的感觉


## 三、源代码
这里除了管线类代码(上面已经给出)都贴出来，3d数学库有所改变，着色器代码没有变化

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

#endif
```
main.cpp:

```
#include <stdio.h>
#include <string>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <assert.h>
#include <fstream>

#include "opengl_math.h"
#include "opengl_pipeline.h"

using namespace std;

GLuint VBO;
GLuint IBO;
GLuint gWorldLocation;

PersProjInfo gPersProjInfo;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

bool ReadFile(const char* pFileName, string &outFile)
{
	ifstream f(pFileName);
	bool ret = FALSE;
	if (f.is_open()) {
		string line;
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
		exit(1);
	}
	return ret;
}

static void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	static float Scale = 0.0f;
	Scale += 0.1f;

	Pipeline p;
	p.Rotate(0.0f, Scale, 0.0f);
	p.WorldPos(0.0f, 0.0f, 5.0f);
	p.SetPerspectiveProj(gPersProjInfo);

	glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, (const GLfloat*)p.GetWPTrans());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);

	glutSwapBuffers();


}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(Render);

	glutIdleFunc(Render);
}
static void CreateVertexBuffer()
{
	Vector3f Vertices[4];

	LoadVector3(Vertices[0], -1.0f, -1.0f, 0.5773f);
	LoadVector3(Vertices[1], 0.0f, -1.0f, -1.15475f);
	LoadVector3(Vertices[2], 1.0f, -1.0f, 0.5773f);
	LoadVector3(Vertices[3], 0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}
static void CreateIndexBuffer()
{
	unsigned int Indices[] = {
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
		fprintf(stderr, "Error creating shader object");
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
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		system("pause");
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

static void ComileShaders()
{
	GLuint ShaderProgram = glCreateProgram();
	if (!ShaderProgram) {
		fprintf(stderr, "Error creating shader program");
		system("pause");
		exit(1);
	}

	string vs, fs;
	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	}

	AddShader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	AddShader(ShaderProgram,fs.c_str(),GL_FRAGMENT_SHADER);

	GLint Success;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, 1024, NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		system("pause");
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, 1024, NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s' \n", ErrorLog);
		system("pause");
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc,char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(1024, 768);
	glutCreateWindow("prespective_projection");

	InitializeGlutCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		system("pause");
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();

	ComileShaders();

	gPersProjInfo.FOV = 30.0f;
	gPersProjInfo.Height = 768;
	gPersProjInfo.Width = 1024;
	gPersProjInfo.zNear = 1.0f;
	gPersProjInfo.zFar = 100.0f;

	

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
	gl_Position = gWorld * vec4(Position,1.0);
	Color = vec4(clamp(Position,0.0,1.0),1.0);
}
```
shader.fs:

```
#version 330

in vec4 Color;

out vec4 FragColor;

void main()
{
	FragColor = Color;
}
```
## 四、运行结果
![image](E:/笔记/Opengl/Perspective_Projection/6.png)

可以看到一个立体感的四面体绕y轴旋转