根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

## 一、索引绘制
我们之前使用glDrawArrays()函数绘制了一个三角形，属于顺序绘制，即从指定的偏移量依次扫描顶点缓冲区所有图元的每一个顶点。

正如下图，我们绘制一个三角形只需要指定三个顶点即可，但是如果绘制两个三角形组成的一个平行四边形就需要六个顶点，而且会有两个顶点相重叠：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/index/image/1.png)

可以看到顶点V2和V3在顶点缓冲区会出现两次，如果我们的图形更复杂一些呢？顶点重复的现象会更多

如果我们引入一个索引缓冲例如下图：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/index/image/2.png)

顶点缓冲区内只放四个顶点，而我们用六个索引就可以表示出按照V1V2V3V4V2V3顺序绘制图形

## 二、代码解释
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
3d数学头文件没有进行改变，因为本节主要是增加了索引绘制，不涉及数学上的多余操作

### 2.2 main.c

```
#include <stdio.h>
#include <string>
#include <fstream>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <assert.h>
#include "opengl_math.h"

using namespace std;

GLuint VBO;
GLuint gWorldLocation;
GLuint IBO;

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
		fprintf(stderr, "%s:%d: unable to opem file '%s'\n", __FILE__, __LINE__, pFileName);
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

	RotationMatrix44(World, Scale, 0, 1, 0);

	glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, &World[0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//绘制前绑定索引缓冲
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//索引绘制图形
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
	GLuint Indices[] = { 0,3,1,
					  1,3,2,
					  2,3,0,
					  0,1,2};
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}
static void AdderShader(GLuint ShaderProgram, const char* ShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (!ShaderObj) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
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

static void CompilerShader()
{
	GLuint ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName,vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName,fs)) {
		exit(1);
	}

	AdderShader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	AdderShader(ShaderProgram,fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		system("pause");
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		system("pause");
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
	glutCreateWindow("Index");

	InitializeGlutCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error:'%s'\n", glewGetErrorString(res));
		system("pause");
		return 1;
	}

	printf("GL version: %s \n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();

	CompilerShader();

	glutMainLoop();

	return 0;
}
```
这里增加了索引绘制的相关代码
#### 2.2.1 创建顶点缓冲区

```
    Vector3f Vertices[4];
	LoadVector3(Vertices[0], -1.0f, -1.0f, 0.0f);
	LoadVector3(Vertices[1], 0.0f, -1.0f, 1.0f);
	LoadVector3(Vertices[2], 1.0f, -1.0f, 0.0f); 
	LoadVector3(Vertices[3], 0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
```
这里在顶点缓冲区里添加了四个顶点数据，想要绘制一个三棱锥，如果看不懂这里的代码请移步教程：
[OpenGL学习之路2----画一个点](https://blog.csdn.net/zach_z/article/details/80045085)

#### 2.2.2 创建索引缓冲器

```
GLuint Indices[] = { 0,3,1,
					  1,3,2,
					  2,3,0,
					  0,1,2};
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
```
- 先是定义一个索引数组，存放与顶点缓冲区中相匹配的位置索引
- IBO是GLuint类型的全局变量，作为索引缓冲区对象的引用句柄

```
GLuint IBO;
```

- 跟创建顶点缓冲器一样，创建索引缓冲器先调用glGenBuffers()返回未使用的缓存对象的名称，再调用glBindBuffer()激活对象，最后使用glBufferData()函数把索引的信息绑定在IBO对象上
- 唯一不同的是再调用glBindBuffer()函数和glBufferData()函数的缓存对象类型是GL_ELEMENT_ARRAY_BUFFER

#### 2.2.3 使用索引绘制图形
渲染函数Render()中，不再用glDrawArrays()来绘制图形，而是用glDrawElements()用索引绘制图形

```
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
```
- 使用索引缓冲之前需要用glBindBuffer()来绑定索引缓冲，每次使用缓冲时都要先进行绑定这点在之前的学习中已经提到过了

```
void glDrawElements (GLenum mode, GLsizei count ,
GLenum type,const GLvoid *indices );
```
- 使用count个元素来定义一系列几何图元，这里一共是12个索引共绘制12个顶点所以是12
- indices定义了元素数组缓存中的偏移地址，这里索引数组是顺序存储没有偏移值所以为0
- mode必须是图元类型的标识符：比如GL_TRIANGLES、GL_LINE_LOOP、GL_LINES、GL_POINTS，这里绘制三角形所以是GL_TRIANGLES

- type必须是GL_UNSIGNED_BYTE,
GL_UNSIGNED_SHORT或者GL_UNSIGNED_INT当中一个,索引数组数据类型为GLuint所以是GL_UNSIGNED_INT

### 2.3 着色器
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
都没有变化，因为这节只是添加索引绘制相关代码，着色器代码跟上一节相同

## 三、运行结果

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/index/image/3.png)

可以看到颜色渐变的三棱锥，围绕着y轴进行旋转，如果不理解为何颜色渐变或者旋转请移步教程：

[OpenGL学习之路6----平移，旋转和缩放变换](https://blog.csdn.net/zach_z/article/details/80072929)

[OpenGL学习之路7----插值](https://blog.csdn.net/zach_z/article/details/80082957)
