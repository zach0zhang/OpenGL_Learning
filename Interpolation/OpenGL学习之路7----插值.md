根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

## 一、插值
- 插值是3D渲染管线中非常重要的部分，其实就是光栅化程序执行的从顶点着色器传过来的变量。

- 光栅化程序对三角形三个顶点之间执行插值处理：对三个顶点之间的每个像素执行片元着色器(即fragment shader),片元着色器通过由顶点着色器传来的插值变量确定每个像素的颜色，光栅化程序将颜色存放到颜色缓冲区中。
- 两种常见的依赖这种插值的变量是三角形法线和纹理坐标，这两个变量都是在顶点着色器中计算完成后传到片元着色器。(法线在以后会用在灯光效果中，纹理坐标用来将纹理覆盖在我们的图形上，都在以后会学习到)

**这里可以简单对插值理解为：在顶点着色器(Vertex Shader)中计算出来的一个值，并从顶点着色器中传出，在片元着色器(Fragment Shader)中接收到这个值并使用它**

## 二、程序解释
### 2.1 opengl_math.h:

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
	MatrixMultiply44(m,temp, rx);

}
//平移变换
inline void TranslationMatrix44(Matrix44f m, float x, float y, float z)
{
	LoadIdentity44(m); m[12] = x; m[13] = y; m[14] = z;
}
#endif
```
3d数学库头文件没有变化，跟上一节相同，如果看不太懂请移步上一节
[OpenGL学习之路6----平移，旋转和缩放变换](https://blog.csdn.net/zach_z/article/details/80072929)
这章只是增加了插值变量的使用

### 2.2 main.c

```
#include <stdio.h>
#include <string>
#include<math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include<fstream>
#include<assert.h>
#include "opengl_math.h"

using namespace std;

GLuint VBO;
GLuint gWorldLocation;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

bool ReadFile(const char* FileName, string &outFile)
{
	ifstream f(FileName);
	bool ret = false;

	if (f.is_open()) {
		string line;
		while (getline(f, line)) {
			outFile.append(line);
			outFile.append("\n");
		}
		f.close();
		ret = true;
	}
	else {
		fprintf(stderr, "%s:%d: unable to open file '%s'\n", __FILE__, __LINE__, FileName);
		system("pause");
	}
	return ret;
}

static void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;
	Scale += 0.001f;

	Matrix44f World;
	ScaleMatrix44(World, sinf(Scale), sinf(Scale), sinf(Scale));

	glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, &World[0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
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
	Vector3f Vertices[3];
	LoadVector3(Vertices[0], -1.0f, -1.0f, 0.0f);
	LoadVector3(Vertices[1], 1.0f, -1.0f, 0.0f);
	LoadVector3(Vertices[2], 0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void AdderShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj) {
		fprintf(stderr, "Error creating shader ype %d\n", ShaderType);
		system("pause");
		exit(1);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType,InfoLog);
		system("pause");
		exit(1);
	}
	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{
	GLuint ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
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

	AdderShader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	AdderShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(1024, 768);
	glutCreateWindow("Interpolation");

	InitializeGlutCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		system("pause");
		return 1;
	}

	printf("GL version: %s \n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();

	CompileShaders();

	glutMainLoop();

	return 0;

}
```
主程序也没有什么改变，唯一值得一提的是我们回调的渲染函数中

```
static float Scale = 0.0f;
Scale += 0.001f;

Matrix44f World;
ScaleMatrix44(World, sinf(Scale), sinf(Scale), sinf(Scale));
```
这里可以看出来，我们会让我们绘制的图形进行一个缩放的工作

### 2.3 着色器
#### 2.3.1 shder.vs(顶点着色器)

```
#version 330

layout (location=0) in vec3 Position;

uniform mat4 gWorld;

out vec4 Color;

void main()
{
	gl_Position = gWorld * vec4(Position,1.0);

	Color=vec4(clamp(Position,0.0,1.0),1.0);
}
```
- 可见声明了一个四维的输出向量Color
- Color的值,XYZ三个分量由Position获得，W分量设为1.0
- clamp()这个内置函数保证数值在0.0到1.0范围之内
- 三角行左下角顶点坐标是(-1,-1)，使用clamp()函数会被转化乘0，所以左下角会呈现黑色

#### 2.3.1 shder.fs(片元着色器)

```
#version 330

in vec4 Color;

out vec4 FragColor;

void main()
{
	FragColor=Color;
}
```
- 声明了一个输入四维向量Color，即是接收刚才顶点着色器传来的输出变量
- FragColor=Color把这个颜色变量直接传递下去

## 三、运行结果
![image](E:/笔记/Opengl/Interpolation/1.png)

可以看到三角形根据位置不同呈现不同的颜色，并且不断的缩放放大