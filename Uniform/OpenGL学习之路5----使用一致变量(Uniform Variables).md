根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

## 一、uniform 存储限制符
- 在着色器运行之前，uniform修饰符可以指定一个在应用程序中设置好的变量，**它不会在图元处理过程中发生变化**
- uniform变量在所有可用的着色阶段之间都是共享的，**它必须定义为全局变量**
- 任何类型的变量(包括结构体和数组)都可以设置为uniform变量
- 着色器无法写入到uniform变量，也无法改变它的值

example:在着色器中声明一个uniform变量
```
uniform float gScale; 
```
在着色器中可以根据名字引用gScale这个变量，如果需要在用户应用程序中设置它的值，则需要两步：
1. 使用glGetUniformLocation()函数获得gScale在列表中的索引
2. 通过glUniform*()或者glUniformMatrix*()系列函数来设置uniform变量的值(gScale是基本float类型的变量所以用glUniform1f())


简单来说，Uniform变量就是应用程序用来向着色器中传递的一个类型和着色器运行时不变的值。

## 二、代码解释
### 2.1 opengl_math:

```
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
```
跟上一节没有区别，因为这一节只是增加了uniform存储限制符的应用

### 2.2 main.c:
```
#include <stdio.h>
#include <string>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <assert.h>
#include "opengl_math.h"

using namespace std;

GLuint VBO;
GLint gScaleLocation; //位置中间变量

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

bool ReadFile(const char* pFileName, string &outFile)
{
	ifstream f(pFileName);

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
		fprintf(stderr, "%s:%d: unable to open file `%s`\n", __FILE__, __LINE__, pFileName);
	}
	return ret;
}


static void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;
	Scale += 0.01f;
	//将值传递给shader
	glUniform1f(gScaleLocation, sinf(Scale));

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

	//将渲染回调注册为全局闲置回调
	glutIdleFunc(Render);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
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
		fprintf(stderr, "Error compiling shader type %d: %s\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}



static void CompileShaders()
{
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	}

	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	}

	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
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

	//查询获取一致变量的位置
	gScaleLocation = glGetUniformLocation(ShaderProgram, "gScale");
	//检查错误
	assert(gScaleLocation != 0xFFFFFFFF);
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

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Uniform");

	InitializeGlutCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();

	CompileShaders();

	glutMainLoop();

	return 0;
}
```
这里只看相比于上一节不同的部分。

### 2.2.1 全局闲置回调函数
注册绘图函数之后增加了一句
```
glutIdleFunc(Render);
```
glutIdleFunc()用于设置全局闲置回调函数，即在程序空闲的时候(没有窗口事件到达时)调用函数Render()

### 2.2.2 获取Uniform变量的位置

```
//查询获取一致变量的位置
gScaleLocation = glGetUniformLocation(ShaderProgram, "gScale");
//检查错误	assert(gScaleLocation != 0xFFFFFFFF);
```
正如之前提到过，要想在应用程序中设置uniform变量的值，第一步先要找到uniform变量在列表中的索引

```
GLint glGetUniformLocation (GLuint program ,
constchar* name);

```
返回着色器程序中uniform变量name对应的索引值，如果name与启用的着色器程序中的**所有uniform变量都不相符，或者name是一个内部保留的着色器变量的名称，返回值为-1**

```
assert(gScaleLocation != 0xFFFFFFFF);
```
如果返回的是-1，则向stderr打印一条信息，然后通过调用 abort 来终止程序运行(即报错和终止程序)

### 2.2.3 设置Uniform变量的值
```
static float Scale = 0.0f;
Scale += 0.01f;
glUniform1f(gScaleLocation, sinf(Scale));
```
在Render()渲染函数中，通过glUniform1f,传送一个不断增大的静态浮点数的sin值，即sinf(Scale)的值在-1到1之间循环改变

### 2.3 shader
shader.vs:
```
#version 330

layout(location = 0) in vec3 Position;

uniform float gScale;

void main()
{
	gl_Position = vec4(gScale * Position.x, gScale * Position.y, Position.z, 1.0);
}

```
顶点着色器中使用uniform变量，并且通过它，改变三角形的三个顶点的位置
- uniform float gScale:声明gScale变量
- gl_Position = vec4(gScale * Position.x, gScale * Position.y, Position.z, 1.0): 通过使用gScale名字来使用gScale变量

shader.fs:

```
#version 330

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0,0.0,0.0,1.0);
}

```
片元着色器没有变化

## 三、运行结果

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Uniform/image/1.png)
可以看到三角形从大变小再由小变大，并且每一次从大变小会翻转位置，因为sinf(Scale)的值是-1到1之间变化
