根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

之前已经学习了怎么将物体在3维空间里自由地缩放、旋转和平移，博客链接如下：
[OpenGL学习之路6----平移，旋转和缩放变换](https://blog.csdn.net/zach_z/article/details/80072929)

[OpenGL学习之路9----混合变换](https://blog.csdn.net/zach_z/article/details/80086523)

也学习怎么把三维世界表现在我们屏幕这个2维平面上，博客链接如下：

[OpenGL学习之路10----透视投影](https://blog.csdn.net/zach_z/article/details/80087986)

**但是我们发现了一个问题，我们默认的视点(就是我们观察的地点)都是在原点并且看向Z轴的正方向，我们希望能够从三维空间的任意地方观察我们的物体，这次我们就学习如何任意移动我们的观察地点(称作相机)**

## 一、数学基础
### 1.1 点积
#### 1.1.1 几何定义：
    设二维空间内有两个向量a和b，它们的夹角为φ(0≤φ≥π)，则点积为：
```
a · b= ||a|| * ||b|| * cosφ
```
- 向量a和b点积的值即为a的长度乘b的长度乘夹角的余弦值(这个定义只对二维和三维空间有效)
- **如果a和b向量都为单位向量，则点积的结果为夹角的余弦值**

定义向量a到向量b的投影为
```
a -> b
```
示意图如下：
![image](E:/笔记/Opengl/Camera_Space/1.png)
则可以推出以下公式：

```
a -> b = ||a|| * cosφ = a · b / ||b||
```
可以看到如果b是单位向量，则点积即是向量a到向量b的投影

#### 1.1.2 代数定义
二维空间中的两个向量a=(xa,ya)和b=(xb,yb),根据
```
X · X = Y · Y = 1
X · Y = 0
```
可以得到a和b的点积为：
```
a · b = (xaX+yaY) · (xbX+ybY)
      = xaxb(X·X)+xayb(X·Y)+xbya(Y·X)+yayb(Y·Y)
      = xaxb + yayb
```
可以推出三维空间a=(xa,ya,za),b=(xb,yb,zb)向量的点积为

```
a · b = xaxb + yayb + zazb
```
### 1.2 叉积
#### 1.2.1 叉积定义
叉积定义为:
```
a × b = a*b*sinθ
```
- 叉积a × b返回一个三维向量，该向量与另外两个向量a和b都正交长度为：

```
||a × b|| = ||a|| * ||b|| * sinθ 
```
![image](E:/笔记/Opengl/Camera_Space/2.png)

三维向量a=(xa,xb,xz),b=(xb,yb,zb)的叉积为

```math
a * b = (x_aX+y_aY+z_aZ) * (x_bX+y_bY+z_bZ)
=(y_az_b-z_ay_b,z_ax_b-x_az_b-x_az_b,x_ay_b-y_ax_b)
```
#### 1.2.2 叉积代码表示

```
inline void CrossProduct3(Vector3f result, const Vector3f u, const Vector3f v)
{
	result[0] = u[1] * v[2] - v[1] * u[2];
	result[1] = u[2] * v[0] - v[2] * u[0];
	result[2] = u[0] * v[1] - v[0] * u[1];
}
```
result存放叉积后的结果(即与向量v和u都正交的向量)
### 1.3 向量单位化代码中表示
非零向量除以它的模(长度)就是单位向量
```
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
//向量单位化
inline void NormalizeVector3(Vector3f u)
{
	ScaleVector3(u, 1.0f / GetVectorLength3(u));
}
```
可以看到向量单位化的代码表示

## 二、UVN相机系统
### 2.1 UVN相机系统的概念
如图所示UVN相机在世界坐标系下的表示：
![image](E:/笔记/Opengl/Camera_Space/3.png)
- N:相机目标朝向的向量
- V：相机竖直向上的向量
- U：指向相机右方的向量
- **如果我们知道了两个向量就可以通过两个向量的叉积得到第三个向量**

### 2.2 世界空间转换到相机空间
假如一个物体在(0,0,5), 相机在(0,0,1)并且相机朝向物体(即朝向z轴方向),我们把物体和相机都向原点移动一个单位，此时物体在(0,0,4)而相机在(0,0,0),但是它们之间的相对距离和方向保持不变。 

**所以我们在保持物体和相机距离和相机方向不变的条件下，移动物体，使得相机移动到原点上**

#### 2.2.1 相机移动
相机移动则是把物体向反方向移动例如下图从y轴向负方向看把相机移动到(1,0,0)，又要保持物体位置相对不变的情况下把相机移回原点，则把物体按反方向移动到(-1,0,0):
![image](E:/笔记/Opengl/Camera_Space/4.png)
可以看出如果相机从原点移动到(x,y,z)，相应物体的变换矩阵应该是
![image](E:/笔记/Opengl/Camera_Space/5.png)

#### 2.2.2 相机旋转
![image](E:/笔记/Opengl/Camera_Space/6.png)
- 通过向量在世界空间中的位置(x,y,z)来获得相机坐标系中的位置(x',y',z')
- 通过之前学习的数学基础,知道任意向量a在单位向量b上的投影即是a和b的点积
- 我们做向量(x,y,z)在相机U轴上的单位向量的点积得到的就是x' 其他同理

#### 2.2.3 UVN矩阵
为了将世界空间中的位置转换到相机空间中用UVN向量定义，则将位置向量和UVN向量进行点积操作
![image](E:/笔记/Opengl/Camera_Space/7.png)

UVN在代码中定义：

```
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
```
## 三、代码解释
### 3.1 管线类的扩充
#### 3.1.1 opengl_pipeline.h:

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
		for (unsigned int i = 0; i < 3; i++)
		{
			m_camera.Pos[i] = Pos[i];
			m_camera.Target[i] = Target[i];
			m_camera.Up[i] = Up[i];
		}
	}

	const Matrix44f* GetWorldTrans();
	const Matrix44f* GetProjTrans();
	const Matrix44f* GetViewTrans();
	const Matrix44f* GetWPTrans();
	const Matrix44f* GetVPTrans();
	const Matrix44f* GetWVPTrans();

};

#endif 
```
管线类相比较上一节又扩充了一些变量和方法，向了解这个类作用的请移步相关博客：
[OpenGL学习之路9----混合变换](https://blog.csdn.net/zach_z/article/details/80086523)
- 这里主要增加了一个私有结构体变量m_camera，里面三个变量分别代表：Pos(相机在世界坐标中位置),Target(相机的朝向向量)，Up(相机竖直朝向向量)
- 与上一节相比增加了三个4*4矩阵变量和三个相应的方法，分别是

```
	Matrix44f m_Vtransformation;  // 相机变换矩阵
	Matrix44f  m_VPtransformation;  //相机变换矩阵*透视投影矩阵
	Matrix44f m_WVPtransformation;  //混合变换矩阵*相机变换矩阵*透视投影矩阵
	const Matrix44f* GetViewTrans();  //获得相机变换矩阵
	const Matrix44f* GetVPTrans();    //获得相机变换矩阵*透视投影矩阵的结果矩阵
	const Matrix44f* GetWVPTrans();  //获得混合变换矩阵*相机变换矩阵*透视投影矩阵的结果矩阵
```
- 还有一个接口函数SetCamera()用来调用设置内部相机结构体里面变量的值

#### 3.1.2 opengl_pipeline.c:

```
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

```
- 可以看到新加的三个方法的实现，分别最后得到相应的变换矩阵
- GetViewTrans()用来得到相机矩阵，可以看到代码中实现：

```
TranslationMatrix44(CameraTranslationTrans, -m_camera.Pos[0], -m_camera.Pos[1], -m_camera.Pos[2]);
CameraMatrix44(CameraRotateTrans, m_camera.Target, m_camera.Up);
MatrixMultiply44(m_Vtransformation, CameraRotateTrans, CameraTranslationTrans);
```
可以看到先进行了一个朝相机位置的反方向平移动作，这个在上面已经做了分析，只后根据相机的U,V两个向量得到UVN矩阵，把两个矩阵相乘得到相机矩阵
- 剩下两个方法只是把之前的混合变换矩阵和透视投影矩阵加了进来获得一个最终矩阵

### 3.2 其他代码
#### 3.2.1 main.cpp:

```
#include <stdio.h>
#include <string>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <assert.h>

#include "opengl_math.h"
#include "opengl_pipeline.h"

#define Window_Width 1024
#define Window_Height 768

using namespace std;

GLuint VBO;
GLuint gWVPLocation;
GLuint IBO;

PersProjInfo gPersProjInfo;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

bool ReadFile(const char* FileName, string &outFile)
{
	ifstream f(FileName);
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
		fprintf(stderr, "%s:%d unable to open file: %s\n", __FILE__, __LINE__, FileName);
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
	p.WorldPos(0.0f, 0.0f, 3.0f);

	//相机变换
	Vector3f CameraPos, CameraTarget, CameraUp;
	LoadVector3(CameraPos, 2.0f, 0.0f, -4.0f);
	LoadVector3(CameraTarget, 0.0f, 0.0f, 1.0f);
	LoadVector3(CameraUp, 0.0f, 1.0f, 0.0f);
	p.SetCamera(CameraPos, CameraTarget, CameraUp);

	p.SetPerspectiveProj(gPersProjInfo);

	glUniformMatrix4fv(gWVPLocation, 1, GL_FALSE, (const GLfloat*)p.GetWVPTrans());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}

static void InitializeGlutCallback()
{
	glutDisplayFunc(Render);

	glutIdleFunc(Render);
}

static void CreateVertexBuffer()
{
	Vector3f Vertices[4];
	LoadVector3(Vertices[0], -1.0f, -1.0f, 0.5773f);
	LoadVector3(Vertices[1], 0.0f, -1.0f, -0.5f);
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
static void AddShader(GLuint ShaderProgram, const char* Shadertext, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj) {
		fprintf(stderr, "Error creating shader object");
		system("pause");
		exit(1);
	}

	const GLchar* p[1];
	p[0] = Shadertext;
	GLint Length[1];
	Length[0] = strlen(Shadertext);
	glShaderSource(ShaderObj, 1, p, Length);
	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader object: '%s'\n", InfoLog);
		system("pause");
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompilShaders()
{
	GLuint ShaderProgram = glCreateProgram();
	if (!ShaderProgram) {
		fprintf(stderr, "Error creating shader program ");
		system("pause");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName,vs)) {
		exit(1);

	}
	if (!ReadFile(pFSFileName,fs)) {
		exit(1);
	}

	AddShader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	AddShader(ShaderProgram,fs.c_str(),GL_FRAGMENT_SHADER);

	GLint Success;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog),NULL , ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s' ", ErrorLog);
		system("pause");
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s' \n", ErrorLog);
		system("pause");
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
	assert(gWVPLocation != 0xFFFFFFFF);

}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(Window_Width, Window_Height);
	glutCreateWindow("Camera Space");

	InitializeGlutCallback();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error '%s'\n", glewGetErrorString(res));
		system("pause");
		return 1;
	}

	printf("GL version %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();
	
	CompilShaders();

	gPersProjInfo.FOV = 60.0f;
	gPersProjInfo.Height = Window_Height;
	gPersProjInfo.Width = Window_Width;
	gPersProjInfo.zNear = 1.0f;
	gPersProjInfo.zFar = 100.0f;

	glutMainLoop();

	return 0;
}
```

主程序改动不多仅仅在渲染函数那里加入了相机参数的设置，和传递Uniform值是传GetWVPTrans()函数获得的最终变换矩阵
#### 3.2.2 opengl_math.h

```
#ifndef __OPENGL_MATH_H
#define __OPENGL_MATH_H

#include <math.h>

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
```
3d函数库添加了向量单位化和UVN矩阵的相关函数,这两部分在上面也讲到

#### 3.2.3 着色器代码
着色器代码较上一节没有变化

shader.vs:

```
#version 330

layout (location = 0) in vec3 Position;

uniform mat4 gWVP;

out vec4 Color;

void main()
{
	gl_Position = gWVP * vec4(Position,1.0);
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
![image](E:/笔记/Opengl/Camera_Space/8.png)
修改相机参数就可以改变我们观察的位置了

