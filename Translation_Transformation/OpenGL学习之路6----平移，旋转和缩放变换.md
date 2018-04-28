## 一、准备工作
### 1.1 表示一个矩阵
我们需要在程序中使用矩阵，我们需要用计算机程序语言来表示出来一个矩阵。

我们用数组来表示一个矩阵，并且规定列填充优先，即一个4*4矩阵表示为：

```
//      4 * 4 矩阵：
//      0       4       8       12
//      1       5       9       13
//      2       6       10      14
//      3       7       11      15
typedef float Matrix44f[16];    
```
此时如果要将一个4*4的矩阵设置为单位矩阵可以这样操作：

```
inline void LoadIdentity44(Matrix44f m)
{
	m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f;  m[12] = 0.0f;
	m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f;  m[13] = 0.0f;
	m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
	m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}
```
### 1.2 矩阵相乘
学过线性代数的同学应该都应该记得矩阵的乘法，这里为了更加扎实的进一步学习，先复习一下矩阵的乘法。

百度百科中的讲解：[矩阵乘法](https://baike.baidu.com/item/%E7%9F%A9%E9%98%B5%E4%B9%98%E6%B3%95/5446029?fr=aladdin)

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/1.png)

- 首先要明确只有当矩阵A的列数等于矩阵B的行数时，A和B才可以相乘
- 矩阵C的行数等于矩阵A的行数，C的列数等于B的列数
- 乘积C的第m行第n列的元素等于矩阵A的第m行的元素与矩阵B的第n列对应元素乘积之和

代码种得到两个矩阵相乘的结果：

```
inline void MatrixMultiply44(Matrix44f product, const Matrix44f a, const Matrix44f b)
{
	unsigned int j, k;
	for (unsigned int i = 0; i < 16; i++) {
		j = i % 4;
		k = i / 4 * 4;
		product[i] = a[j] * b[k] + a[j + 4] * b[k + 1] + a[j + 8] * b[k + 2] + a[j + 12] * b[k + 3];
	}
}
```
### 1.3 角度和弧度
弧度的定义：弧长等于半径的弧，其所对的圆心角为1弧度

一周的弧度是2π，一周的角度是360°

弧度= 角度 * ( π / 180 )

角度 = 弧度 * ( 180 / π ) 

我们在程序中使用的sinf(),cosf(),tanf()参数都是弧度，如果我们使用角度，则需要把角度转化为弧度

代码中角度弧度互相转化
```
#define PI (3.14159265358979323846)
#define PI_DIV_180 (0.017453292519943296)
#define INV_PI_DIV_180 (57.2957795130823229)

#define DegToRad(x)  ((x)*PI_DIV_180)
#define RadToDeg(x)  ((x)*INV_PI_DIV_180)
```



## 二、平移变换
平移变换示意图如下：
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/2.png)

我们的任务便是**找到一个矩阵M(称为平移矩阵)，对于给定的点P(x,y,z)和平移向量V(v1,v2,v3)，使得M\*P=P1(x+v1,y+v2,z+v3)**

最后通过种种努力，发现了一个4 * 4的矩阵可以达到这种效果：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/3.png)

- 像这样使用4维向量表示一个三维向量叫做齐次坐标
- 4维向量的四个分量分别是X、Y、Z、W，之前看到的着色器中的内部变量gl_Position就是一个四维向量
- 通常表示点的矩阵让W=1,表示向量的矩阵让W=0，因为点可以被做变换而向量不可以
- **总之先不要管不懂的地方，只要知道平移矩阵我们已经找到了，我们只需要在程序中把平移矩阵与我们的图形的位置坐标相乘就能得到平移后的图形了**

在代码中设置一个平移矩阵：

```
inline void TranslationMatrix44(Matrix44f m, float x, float y, float z)
{
	LoadIdentity44(m); m[12] = x; m[13] = y; m[14] = z;
}
```
## 三、旋转变换
有了刚才平移变换的基础，我们知道了旋转变换其实也就是找到一个旋转矩阵，来与我们位置坐标相乘能得到旋转后的位置坐标
### 3.1 推导
旋转变换示意图如下：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/4.png)

示意图为从z轴的正方向朝负方向看，从位置1旋转到位置2(即从(x1,y1)到(x2,y2))，如果圆的半径为1(即是单位圆),则可以得到：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/5.png)

再由高中数学三角函数公式可以推导：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/6.png)

此时找到了旋转α度角之后的x,y坐标，此时只需要找到旋转矩阵与原坐标向量相乘等于新的坐标向量即可

### 3.2 旋转矩阵
**绕z轴旋转(z轴不变)：**

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/7.png)

**绕y轴旋转(y轴不变)：**

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/8.png)

**绕x轴旋转(x轴不变)：**

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/9.png)

代码设置一个旋转矩阵：

```
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
```
## 四、缩放变换
缩放变换可以由平移矩阵推导出来
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/10.png)
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/11.png)

代码设置一个缩放矩阵：

```
inline void ScaleMatrix44(Matrix44f m, float xScale, float yScale, float zScale)
{
	LoadIdentity44(m); m[0] = xScale; m[5] = yScale; m[10] = zScale;
}
```

## 五、代码解释
### 5.1 opengl_math.h:

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
这次我们自创的3d函数头文件中，增加了4*4矩阵和对其的相关操作(所有操作在上面已经解释过)，都定义为内联函数，如果需要引用则只需给好参数在主程序中引用即可。

### 5.2 main.c:

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
GLint gWorldLocation1;
GLint gWorldLocation2;
GLint gWorldLocation3;

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

	Matrix44f World1;
	Matrix44f World2;
	Matrix44f World3;

	TranslationMatrix44(World1, sinf(Scale), 0.0f, 0.0f);

	RotationMatrix44(World2, Scale, 0, 0, 1);

	Vector3f vScale;
	LoadVector3(vScale, sinf(Scale), sinf(Scale), sinf(Scale));
	ScaleMatrix44(World3,vScale[0], vScale[1], vScale[2]);

	glUniformMatrix4fv(gWorldLocation1, 1, GL_FALSE, &World1[0]);//GL_FLASE: 数组列优先
	glUniformMatrix4fv(gWorldLocation2, 1, GL_FALSE, &World2[0]);
	glUniformMatrix4fv(gWorldLocation3, 1, GL_FALSE, &World3[0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
	glutSwapBuffers();
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

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
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
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
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
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName,vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName,fs)) {
		exit(1);
	}

	AddShader(ShaderProgram, vs.c_str(),GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(),GL_FRAGMENT_SHADER);

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
	
	gWorldLocation1 = glGetUniformLocation(ShaderProgram, "gWorld1");
	assert(gWorldLocation1 != 0xFFFFFFFF);

	gWorldLocation2 = glGetUniformLocation(ShaderProgram, "gWorld2");
	assert(gWorldLocation2 != 0xFFFFFFFF);
	
	gWorldLocation3 = glGetUniformLocation(ShaderProgram, "gWorld3");
	assert(gWorldLocation3 != 0xFFFFFFFF);
}


static void InitializeGlutCallbacks()
{
	glutDisplayFunc(Render);

	glutIdleFunc(Render);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Transformation");

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

	CompileShaders();

	glutMainLoop();

	return 0;


}
```
这里只说明与上一节不一样的地方
#### 5.2.1 获取Uniform变量的位置

```
gWorldLocation1 = glGetUniformLocation(ShaderProgram, "gWorld1");
assert(gWorldLocation1 != 0xFFFFFFFF);
gWorldLocation2 = glGetUniformLocation(ShaderProgram, "gWorld2");
assert(gWorldLocation2 != 0xFFFFFFFF);
gWorldLocation3 = glGetUniformLocation(ShaderProgram, "gWorld3");
assert(gWorldLocation3 != 0xFFFFFFFF);
```
这次我们声明了三个uniform变量，分别代表平移矩阵、旋转矩阵和缩放矩阵，所以这里需要使用glGetUniformLocation()函数三次，分别找到三个uniform变量在列表中的索引

#### 5.2.2 设置Uniform变量的值

```
    Matrix44f World1, World2,World3;
	TranslationMatrix44(World1, sinf(Scale), 0.0f, 0.0f);

	RotationMatrix44(World2, Scale, 0, 0, 1);

	Vector3f vScale;
	LoadVector3(vScale, sinf(Scale), sinf(Scale), sinf(Scale));
	ScaleMatrix44(World3,vScale[0], vScale[1], vScale[2]);

	glUniformMatrix4fv(gWorldLocation1, 1, GL_FALSE, &World1[0]);//GL_FLASE: 数组列优先
	glUniformMatrix4fv(gWorldLocation2, 1, GL_FALSE, &World2[0]);
	glUniformMatrix4fv(gWorldLocation3, 1, GL_FALSE, &World3[0]);
```
- 这里引用了内联函数TranslationMatrix44()来得到平移矩阵，并且是在x轴上做平移运动，从x轴的-1到1
- 引用RotationMatrix44()得到旋转矩阵，并且是绕z轴旋转
- ScaleMatrix44()得到缩放矩阵
- 这里因为要传送一个4*4的矩阵Uniform变量值到着色器中,所以使用glUniformMatrix4fv()函数

```
void glUniformMatrix {234}{fd}v(GLint location , GLsizei count ,
GLboolean transpose,
const GLfloat * values );
```
- location是索引位置，载入count个数据的集合，transpose设置数组行优先还是数组列优先,values是要传入的值的地址
- 因为我们的数组定义的是列填充优先，所以第三个参数要设置为GL_FALSE
- 这里我们用一个长度为16的数组传递16个float值作Uniform变量的值,所以第二个参数为1，第四个参数是&Worldx[0]

### 5.3 着色器
#### 5.3.1 shader.vs:

```
#version 330

layout (location=0) in vec3 Position;

uniform mat4 gWorld1;
uniform mat4 gWorld2;
uniform mat4 gWorld3;


void main()
{
	gl_Position =  gWorld1*gWorld2*gWorld3 * vec4(Position,1.0);
}
```
可以看到这次声明了三个4*4的矩阵变量，分别从程序中接收平移矩阵，旋转矩阵和缩放矩阵的值。
并且内置变量gl_Position最后的值是三个矩阵相乘后再乘顶点坐标，即先平移再旋转再缩放(这里先这么理解吧，这里只是学习图形变换)
#### 5.3.1 shader.fs:

```
#version 330

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0,0.0,0.0,1.0);
}
```
片元着色器没有变化，还是让渲染的图形颜色为红

## 六、运行结果
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Translation_Transformation/image/12.png)
可以看到红色的三角形在x轴的-1到1间来回移动，并且在xy平面上旋转，并且由大变小再由小变大做缩放运动
