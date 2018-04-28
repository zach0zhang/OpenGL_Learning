根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程
## 一、OpenGL 渲染管线
这节相比上一节有了本质上的区别，OpenGL实际上是通过渲染管线(rendering pipeline),经过一系列的数据处理，将应用程序的数据转换到最终渲染的图像。

在《OpenGL Programming Guide 9th》讲解了渲染管线，下图即是OpenGL 4.5版本的管线：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Shader/image/2.png)

- Vertex Data(顶点数据)：OpenGL将所有数据保存到缓存对象当中,正如上节当中的glVertexAttribPointer()函数所做的工作，并调用glDrawArrays()函数请求渲染几何图元
- Vertex Shader(顶点着色器)：接受在顶点缓存对象中给出的顶点数据，独立处理每个顶点(对于绘制命令传输的每个顶点，OpenGL都会调用一个顶点着色器来处理顶点的相关数据)。**这个阶段是必须的**
- Tessellationj shading stage(细分着色阶段)：这个阶段是由Tessellation Control Shader(细分控制着色器)和Tessellation Evaluation Shader(细分赋值着色器)完成的。 这个阶段启用之后，会收到来自顶点着色阶段的输出数据，并对收到的顶点进行进一步的处理，它会在OpenGL管线内部生成新的几何体。**这是一个可选阶段**
- Geometry Shader(几何着色器)：它会在OpenGL管线内部对所有几何图元进行修改，可以选择输入图元生成更多的几何体，改变几何图元的类型(将三角形转化乘线段之类)，或者放弃所有的几何体。**这是一个可选阶段**
- Primitive Setup(图元装配)：之前着色阶段处理的都是顶点数据，此外，这些顶点构成几何图元的所有信息也会被传递到OpenGL当中。图元装配阶段将这些顶点与相关的几何图元之间组织起来，准备下一步的剪切和光栅化工作
- Culling and Clipping(裁剪和剪切)：顶点可能落在视口之外(即我们能够绘制的窗口区域)，此时顶点相关的图元会做出改动，保证相关像素不会绘制在视口以外。**由OpenGL自动完成**
- Rasterization(光栅化)：光栅化是判断某一部分几何体(点、线或者三角形)所覆盖的屏幕空间。因为屏幕是由一个个的像素点构成的，如果要画一条线，就要判断这条线在哪几个像素点表示，配合下图理解：
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Shader/image/3.png)
- Fragment Shader(片元着色器)：最后一个可以通过编程控制编程控制屏幕上显示颜色的阶段叫做片元着色阶段。这个阶段处理OpenGL光栅化之后生成的独立片元，使用着色器计算片元的最终颜色和它的的深度值。**这个阶段是必须的**

## 二、GLSL构建顶点着色器和片元着色器
GLSL是OpenGL的着色语言，跟使用C语言作为基础高阶着色语言，通过了解渲染管线我们知道了顶点着色器和片元着色器必不可少的，所以我们用GLSL语言构建顶点着色器和片元着色器。

shader.vs(顶点着色器)：

```
#version 330

layout (location = 0) in vec3 Position;

void main()
{
    gl_Position = vec4(0.5 * Position.x, 0.5 * Position.y, Position.z, 1.0);
}
```
shader.fs(片元着色器)：

```
#version 330

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
```

```
#version 330
void main()
{
    // code
}
```
- 每一个着色器程序与C程序类似，都是从main()函数开始
- #version 这个预处理命令设置当前使用GLSL版本名称，这里#version 330 代表使用3.3版本

```
layout (location = 0) in vec3 Position;
```
- layout(location=0) 是一个布局限定符，作用是把缓冲区里索引的数据绑定到我们输入和输出变量上，即glVertexAttributePointer()函数第一个参数所代表的索引
- in vec3 Position：意思是声明一个三个浮点数的输入变量为Position

```
gl_Position = vec4(0.5 * Position.x, 0.5 * Position.y, Position.z, 1.0);
```
- gl_Position是一个内置变量，作为输出变量，用来保存顶点位置的齐次坐标，即就是顶点着色器输出的顶点位置信息存储的地方
- gl_Position是一个四维向量，需要4个分量，所以用vec4()构造，而参数是刚才从顶点缓冲区传来的Position的xyz三个分量的一半，和1.0一起构成，第四个分量W其实代表透明度，设为1.0即不透明。

```
out vec4 FragColor;

FragColor = vec4(1.0, 0.0, 0.0, 1.0);
```
- 片段着色器里面声明了一个四维输出向量(float类型)FragColor,用来输出最后的颜色值
- 直接通过vec4(1.0,0.0,0.0,1.0)赋值给FragColor，红(R/X)：1.0,绿(G/Y)：0.0，蓝(B/Z)：0.0, 透明度(W): 1.0,所以是红色不透明

**通过上面的学习，应该对顶点着色器和片元着色器有了一个直观的感受： 顶点着色器决定要绘制图形的位置，片元着色器决定要绘制图形的颜色**

## 三、代码解释
opengl_math.h
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
这段数学头文件没有改变，因为还是画一个三角形，只是这回用到了着色器，以后会有很多东西要往上添加

main.c:

```
#include <stdio.h>
#include <string>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include "opengl_math.h"

using namespace std;
GLuint VBO;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";



static void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

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
		fprintf(stderr, "%s:%d: unable to open file `%s`\n", __FILE__,__LINE__,pFileName);
	}
	return ret;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	//check if it is successful
	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	//define shader code source
	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	//Compiler shader object
	glCompileShader(ShaderObj);

	//check the error about shader
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	//bound the shader object to shader program
	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompilerShaders()
{
	//Create Shaders
	GLuint ShaderProgram = glCreateProgram();
	
	//Check yes or not success
	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	//the buffer of shader texts
	string vs, fs;
	//read the text of shader texts to buffer
	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	}

	//add vertex shader and fragment shader
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	//Link the shader program, and check the error
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram,GL_LINK_STATUS,&Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	//check if it can be execute
	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	//use program
	glUseProgram(ShaderProgram);

}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Shader");

	InitializeGlutCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();

	CompilerShaders();

	glutMainLoop();

	return 0;
}
```
### 3.1 编译着色器
```
CompilerShaders();
```
main()函数里多了一个CompilerShaders()函数，这个函数是我们自定义的，用来完成着色器的编译工作

**我们主要任务是创建GLSL着色器对象，编译和链接来生成可执行着色器程序。下图给出了具体过程**

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Shader/image/4.png)

### 对于每个着色器对象，我们都需要进行以下步骤
#### (1) 创建一个着色器对象

```
GLuint ShaderObj = glCreateShader(ShaderType);
	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
```
GLuint glCreateShader(GLenum type)用来创建一个着色器对象，type必须是

type  | 说明
---|---
GL_VERTEX_SHADER | 顶点着色器
GL_FRAGMENT_SHADER | 片元着色器
GL_TESS_CONTROL_SHADER|细分控制着色器
GL_TESS_EVALUATION_SHADER|细分赋值着色器
GL_GEOMETRY_SHADER|几何着色器
GL_COMPUTE_SHADER|计算着色器

#### (2) 将着色器源代码编译为对象

```
const GLchar* p[1];
p[0] = pShaderText;
GLint Lengths[1];
Lengths[0] = strlen(pShaderText);
glShaderSource(ShaderObj, 1, p, Lengths);
glCompileShader(ShaderObj);
```

```
glShaderSource((GLuint shader , GLsizei count ,
const GLchar **string ,const GLint *length) //将着色器源代码关联到一个着色器对象上
```

- 第一个参数是着色器对象
- 第二个参数是两个数组的元素个数，这里只有一个
- 第三个参数是源代码数据(实际是一个长度为count的数组，数组里每个元素都是一个字符串)
- 第四个参数是对应源代码的长度(实际上就是长度count的数组里面每个元素字符串的长度)
- 这里为了简化操作就用了一个字符串保存所有的shader源代码，用一个整形数组保存了源代码长度
- **这里解释的比较绕，不知道怎么很好的表达，如果不想深究，就只用后面两个参数记住一个是源代码，一个是源代码长度即可**

```
glCompileShader(ShaderObj);
```
- glCompileShader():即编译着色器对象

#### (3) 验证着色器编译是否成功

```
GLint success;
glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
```
- 使用glGetShaderiv，并且第二个参数为GL_COMPILE_STATUS即返回编译过程的状态
- glGetShaderInfoLog((GLuintshader , GLsizei bufSize,
GLsizei *length ,char *infoLog):返回shader的编译结果，返回一个以NULL结尾的字符串，保存在infoLog缓存中，长度为length个字符串。日志返回最大值用bufSize来定义

### 之后要将多个着色器对象链接为一个着色器程序
#### (1) 创建一个着色器程序

```
GLuint ShaderProgram = glCreateProgram()
```
glCreateProgram创建一个空的着色器程序，返回值是一个非零的整数，如果为0则说明发生了错误

#### (2) 将着色器对象关联到着色器程序

```
glAttachShader(ShaderProgram, ShaderObj);
```
将着色器对象Shaderobj关联到着色器程序program上

#### (3) 链接着色器程序

```
glLinkProgram(ShaderProgram);
```
处理所有与ShaderProgram关联的着色器对象来生成一个完整的着色器程序
#### (4) 判断着色器的链接过程是否成功完成

```
glGetProgramiv(ShaderProgram,GL_LINK_STATUS,&Success);
if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}
```
- glGetProgramiv()函数，使用GL_LINK_STATUS参数来查询链接操作的结果，如果返回值是0则错误，通过glGetProgramInfoLog()获取链接日志信息判断错误原因

#### (5)检查当前管线状态，程序是否能被执行

```
glValidateProgram(ShaderProgram);
glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
```
#### (6)使用着色器

```
glUseProgram(ShaderProgram);
```
### 3.2 渲染回调函数

```
glEnableVertexAttribArray(0);
...
glDisableVertexAttribArray(0);
```
- glEnableVertexAttribArray()开启一个顶点的属性，这里参数为0，即开启索引为0的顶点属性，在渲染管线中的顶点着色器“layout (location = 0) in vec3 Position”相对应，只有开启了索引为0的顶点属性，顶点着色色器才能过去缓存区索引为0的数据。
- glDisableVertexAttribArray()跟上面的使能函数相对应，在进行了指定顶点着色器变量与我们存储在缓存对象中数据的关系(即着色管线装配)和绘制命令够，用来禁用索引相关联的数组

### 3.3 读取GLSL源代码

```
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
		fprintf(stderr, "%s:%d: unable to open file `%s`\n", __FILE__,__LINE__,pFileName);
	}
	return ret;
}
```
- 函数输入参数是文件名称和要把源代码存储到的字符串地址
- 函数返回一个布尔类型的值，成功则为TRUE,不成功则为FALSE
- 大概操作就是打开文件，之后一行一行读取源文件，并存储到字符串内

## 运行结果：
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Shader/image/1.png)
