根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

上一篇博文讲述了我们怎么把相机摆在三维空间的任意一个位置，想要了解的同学请移步上一篇博客：
[OpenGL学习之路11----相机空间](https://blog.csdn.net/zach_z/article/details/80102219)

这次我们的目标是通过键盘上的方向键来移动相机。**但是仅仅是改变相机的位置，而并没有改变相机的目标朝向(target向量)和竖直头顶方向(up向量)。**

## 一、键盘操作
### 1.1 回顾显示回调函数
先回顾下我们渲染显示一个图形的时候是怎么做的：
```
glutDisplayFunc(Render);
glutIdleFunc(Render);
```
- 我们把渲染函数Render()注册成为显示回调函数，再把它设置成全局闲置回调函数。因为我们的OpenGL程序跑在一个窗口系统里，大多数工作通过回调函数完成，我们在渲染函数里面还传递了Uniform变量的值，所以要让程序闲置时就不停的执行它。
- **主要是明白我们显示图形是通过回调渲染函数，渲染函数里有绘制命令从而完成的**
### 1.2 注册键盘事件回调
同理我们想要获取键盘值并执行相应操作，我们也要注册一个回调事件，用于在获得键盘按下后执行某些操作。

一般使用两个GLUT接口函数用来注册键盘按下后的回调函数

```
glutSpecialFunc()
glutKeyboardFunc()
```
- glutKeyboardFunc()用于注册常规按键(字母和数字键)触发后的回调函数。我们这次没有用到，所以不做解释。
- glutSpecialFunc()函数用于注册"特殊按键"按下后执行的回调函数
- 在freeglut_std.h中可以看到定义好的特殊按键的值，从名称就可以看出是哪些按键：

```
/*
 * GLUT API macro definitions -- the special key codes:
 */
#define  GLUT_KEY_F1                        0x0001
#define  GLUT_KEY_F2                        0x0002
#define  GLUT_KEY_F3                        0x0003
#define  GLUT_KEY_F4                        0x0004
#define  GLUT_KEY_F5                        0x0005
#define  GLUT_KEY_F6                        0x0006
#define  GLUT_KEY_F7                        0x0007
#define  GLUT_KEY_F8                        0x0008
#define  GLUT_KEY_F9                        0x0009
#define  GLUT_KEY_F10                       0x000A
#define  GLUT_KEY_F11                       0x000B
#define  GLUT_KEY_F12                       0x000C
#define  GLUT_KEY_LEFT                      0x0064
#define  GLUT_KEY_UP                        0x0065
#define  GLUT_KEY_RIGHT                     0x0066
#define  GLUT_KEY_DOWN                      0x0067
#define  GLUT_KEY_PAGE_UP                   0x0068
#define  GLUT_KEY_PAGE_DOWN                 0x0069
#define  GLUT_KEY_HOME                      0x006A
#define  GLUT_KEY_END                       0x006B
#define  GLUT_KEY_INSERT                    0x006C
```
- glutSpecialFunc()注册的函数必须带有三个int变量的参数，第一个参数是按键的键值也就是上面define各个按键的键值，x和y是以左上角为起点按键事件发生时鼠标点所处的位置(本节没用到鼠标)

```
glutSpecialFunc(Keyboard);
void Keyboard(int key,int x,int y );
```
## 二、相机类
为了使我们的程序高内聚低耦合，更加层次分明我们设置一个相机类，用来存放跟相机有关的参数变量和对应的方法

### 2.1 opengl_camera.h:

```
#ifndef __OPENGL_CAMERA_H
#define __OPENGL_CAMERA_H
#include "opengl_math.h"
class Camera
{
private:
	Vector3f m_pos;
	Vector3f m_target;
	Vector3f m_up;

	int m_windowWidth;
	int m_windowHeight;
public:
	Camera(int Window_Width, int Window_Height)
	{
		m_windowWidth = Window_Width;
		m_windowHeight = Window_Height;
		LoadVector3(m_pos,0.0f, 0.0f, 0.0f);
		LoadVector3(m_target,0.0f, 0.0f, 1.0f);
		LoadVector3(m_target,0.0f, 1.0f, 0.0f);
	}
	Camera(int Window_Width, int Window_Height, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
	{
		m_windowWidth = Window_Width;
		m_windowHeight = Window_Height;
		CopyVector3(m_pos, Pos);
		CopyVector3(m_target, Target);
		CopyVector3(m_up, Up);
	}
	const Vector3f& GetPos()
	{
		return m_pos;
	}
	const Vector3f& GetTarget()
	{
		return m_target;
	}
	const Vector3f& GetUp()
	{
		return m_up;
	}
	bool OnKeyboard(int key);
};

#endif
```
- 可以看到私有变量共有五个，这些向量在之前的学习中已经详细解释过了它们的含义，分别是
        

    m_pos: 相机所处世界空间的位置
    m_target: 相机的朝向方向向量
    m_up: 相机的头顶方向向量
    m_windowWidth: 屏幕的宽
    m_windowHeight: 屏幕的高
- 两个类的构造函数，用来初始化相机的参数
- 三个获得相机三个重要参数的接口函数
- 还有一个返回一个布尔类型的方法，从函数名可以看出是键盘操作，具体会在opengl_camera.cpp里实现

### 2.2 opengl_camera.cpp:

```
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
```
- 根据代码可以看到分别对四个方向按键进行了不同的操作
- “上”“下”键按下，相机进行前后移动，即从现在的位置加上或者减去一定量的target向量

```
m_pos[i] += m_target[i] * StepSize;
m_pos[i] -= m_target[i] * StepSize;
```
StepSize为步长，在文件开始时定义，也就是没按一下键移动距离的长度，这里是一个单位
- “左”“右”键按下，相近进行左右移动，**通过target向量和up向量进行叉积来得到与它们两个向量正交的向量(即相机移动的方向向量)**

不明白叉积可以移步上一博客学习：
[OpenGL学习之路11----相机空间](https://blog.csdn.net/zach_z/article/details/80102219)
 
另外进行补充，叉积不满足交换律,a * b和b * a得到的向量正好相反

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Keyboard/image/1.png)

这里可以用右手定则来判断：张开右手，右手根部作为旋转中心，四指方向为叉积左向量，旋转到叉积右向量，大拇指方向即为叉积结构向量方向

#### 这里很坑的地方来了：OpenGL是右手坐标系，而我们的相机空间是左手坐标系

**左手坐标系：伸开左手，大拇指指向X轴正方向，食指指向Y轴正方向，其他三个手指指向Z轴正方向。**

**右手坐标系：伸开右手，大拇指指向X轴正方向，食指指向Y轴正方向，其他三个手指指向Z轴正方向。**

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Keyboard/image/4.png)

**我们两个向量进行叉积后的结果是右手坐标系中的向量，而我们移动相机却是按照左手坐标系开始移动的，所以我们需要反着叉积。**

**比如向量u=(0,1,0)和v=(0,0,1),u * v=(1,0,0)这是右手坐标系乘出来的结果，而我们要得到左手坐标系的结果(-1,0,0)就必须v * u**

- 向左移动方向即为相机头顶方向向量与相机目标方向向量的叉积,而通过刚才的分析知道如果直接叉乘是右手坐标系的，所以要反着叉乘

```
CrossProduct3(Left, m_target, m_up);
NormalizeVector3(Left);
for (int i = 0; i < 3; i++)
	m_pos[i] += Left[i] * StepSize;
```
再把方向向量进行单位化，与我们的步长相乘加到相机位置上
- 向右移动方向为向左方向的反方向，同时也是向左时候两向量进行叉积左右交换后的叉积。

### 2.3 键盘回调函数
main.cpp:
```
...
static void Keyboard(int key,int x,int y )
{
	pGameCamera->OnKeyboard(key);
}
static void InitializeGlutCallback()
{
	glutDisplayFunc(Render);
	glutIdleFunc(Render);

	//注册键盘回调函数
	glutSpecialFunc(Keyboard);
}
...
```
在主程序中注册了键盘回调函数，如果有特殊按键按下，则传按键值进相机类的OnKeyboard()方法里执行相应改变相机位置的操作

## 三、其他代码
3d函数库、main.cpp和着色器代码几乎没有任何改动
### 3.1 opengl_math.h:

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
//向量复制
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
- 唯一增加了一个向量复制的内联函数，因为每次用一个Vector3f给另一个Vector3f赋值太麻烦了，所以直接写了一个复制函数，用到了<string>里的memcpy函数

```
inline void CopyVector3(Vector3f dst, const Vector3f src) 
{ 
    memcpy(dst, src, sizeof(Vector3f)); 
}
```
- 其他代码没有进行改变跟之前一样，因为这节只设计键盘操作和相机位置的移动，并没有其他数学方面的知识加入

### 3.2 main.cpp

```
#include <stdio.h>
#include <string>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <assert.h>
#include <fstream>

#include "opengl_math.h"
#include "opengl_pipeline.h"
#include "opengl_camera.h"

#define Window_Width 1024
#define Window_Height 768
using namespace std;

const char* pSVFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

GLuint VBO, IBO;
GLuint gWVPLocation;

Camera* pGameCamera = NULL;
PersProjInfo gPersProjInfo;

bool ReadFile(const char* FileName,string &outFile)
{
	ifstream f(FileName);
	bool ret = FALSE;

	if (f.is_open()) {
		string line;
		while (getline(f, line)){
			outFile.append(line);
			outFile.append("\n");
		}
		f.close();
		ret = TRUE;
	}
	else {
		fprintf(stderr, "%s:%d: unable to open file : %s", __FILE__, __LINE__, FileName);
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

	p.SetCamera(*pGameCamera);
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
static void Keyboard(int key,int x,int y )
{
	pGameCamera->OnKeyboard(key);
}
static void InitializeGlutCallback()
{
	glutDisplayFunc(Render);
	glutIdleFunc(Render);

	//注册键盘回调函数
	glutSpecialFunc(Keyboard);
}

static void CreateVertexBuffer()
{
	Vector3f Vertices[4];
	LoadVector3(Vertices[0], -1.0f, -1.0f, 0.5773f);
	LoadVector3(Vertices[1], 0.0f, -1.0f, -1.5f);
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
static void AddShader(GLuint ShaderProgram,const char* ShaderText,GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj) {
		fprintf(stderr, "Error creating shader object");
		system("pause");
		exit(1);
	}

	const GLchar* p[1];
	p[0] = ShaderText;
	GLint Length[1];
	Length[0] = strlen(ShaderText);
	glShaderSource(ShaderObj,1,p,Length);
	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader object: '%s'", InfoLog);
		system("pause");
		exit(1);
	}

	glAttachShader(ShaderProgram,ShaderObj);
}
static void CompilShaders()
{
	GLuint  ShaderProgram = glCreateProgram();
	if (!ShaderProgram) {
		fprintf(stderr, "Error creating shader program");
		system("pause");
		exit(1);
	}

	string vs,fs;

	if (!ReadFile(pSVFileName,vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName,fs)) {
		exit(1);
	}

	AddShader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	AddShader(ShaderProgram,fs.c_str(),GL_FRAGMENT_SHADER);

	
	GLchar ErrorLog[1024] = { 0 };
	GLint Success;
	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success){
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'",ErrorLog);
		system("pause");
		exit(1);
	}
	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Vaild to the Shader program '%s'", ErrorLog);
		system("pause");
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
	assert(gWVPLocation!=0xFFFFFFFF);
}


int main(int argc, char **argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(Window_Width,Window_Height);
	glutCreateWindow("Camera Space");

	InitializeGlutCallback();
	
	Vector3f CameraPos, CameraTarget, CameraUp;
	LoadVector3(CameraPos, 0.0f, 0.0f, -4.0f);
	LoadVector3(CameraTarget, 0.0f, 0.0f, 1.0f);
	LoadVector3(CameraUp, 0.0f, 1.0f, 0.0f);
	pGameCamera = new Camera(Window_Width, Window_Height, CameraPos, CameraTarget, CameraUp);

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
- 这里主程序代码也没有什么太大的改变，主要有三点跟相机有关的改变
1. 声明了一个指向相机类对象的指针全局变量
```
Camera* pGameCamera = NULL;
```
2. 在主函数中创建相机类的对象pGameCamera并初始化自定义的相机相关参数
```
Vector3f CameraPos, CameraTarget, CameraUp;
LoadVector3(CameraPos, 0.0f, 0.0f, -4.0f);
LoadVector3(CameraTarget, 0.0f, 0.0f, 1.0f);
LoadVector3(CameraUp, 0.0f, 1.0f, 0.0f);
pGameCamera = new Camera(Window_Width, Window_Height, CameraPos, CameraTarget, CameraUp);
```
这里可以看到相机的位置在世界空间里是(0.0f,0.0f,-4.0f)，相机目标方向为z轴正方向，头顶方向为y轴正方向
3. 并在渲染函数中通过相机类对象pGameCamera设置管线类对象p里的相机参数
```
p.SetCamera(*pGameCamera);
```
### 3.3 着色器
着色器代码没有任何变化

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
打开程序运行截图：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Keyboard/image/2.png)

按两下“上”键和两下“左”键

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Keyboard/image/3.png)

可以看到相机向前移动，并且移动到了左边，因为我们移动相机而物体没动相机向左移，物体便向右移动

