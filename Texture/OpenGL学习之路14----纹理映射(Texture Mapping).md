根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

本节的纹理学习还参考了教程： [learnopengl](https://learnopengl.com/Getting-started/Textures)和《OpenGL 编程指南》

## 一、纹理
### 1.1 纹理映射简介
我们之前的OpenGL学习一直停留在绘制一个彩色的四面体：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Texture/image/1.png)

而现实世界物体的表面一般都呈现丰富特殊的颜色，并且在很小的图形上呈现多彩的变化，如果我们还是用计算机计算出一个个微小像素上的颜色再拼成特定的“图案”是非常辛苦的工作。

**纹理映射就是找到一张图片，然后把它“粘”到物体表面上，就像贴墙纸一样。**

### 1.2 2D纹理
OpenGL支持不同类型的纹理：1D(一维)，2D(二维)，3D(三维)，CUBE(立方体)等等，我们使用的是2D纹理

- 通过之前的学习，我们知道在在渲染管线中，我们会计算出每个要在屏幕上显示的像素点的颜色并显示出来。
- 而我们拿到一张图片，并且要把它“贴”到我们的图形上去，我们要确定把我们图片上哪块部分贴到哪个像素点上
- 所以我们要指定图片的“某一块”，这个时候就需要指定**纹理坐标**

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Texture/image/2.png)

上图显示了纹理坐标怎么定义，纹理坐标定义在**单位化的正方形内，并且以左下角为(0,0)**

    比如图片像素是480*320,纹理坐标是(0.5,0.5),则在图形中的坐标即为(240,160)
- 当我们为顶点提供一些列的纹理坐标，在光栅化阶段，会对纹理坐标进行插值计算，计算出每个对应像素点需要的**纹素(纹理中的一个像素)**
- 纹素中包含一个用于跟屏幕上像素点对应的颜色值，许许多多的纹素里的颜色值显示在我们屏幕上的不同像素上就显示出了我们要的纹理
- 并且纹理坐标和我们绘制的图形的位置坐标对应，下图我们绘制了一个三角形，并用我们的纹理贴图里面的一块“贴”在我们的三角形上当图案，再进行缩放和旋转操作：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Texture/image/3.png)

### 1.3 使用纹理映射
**使用纹理映射，需要遵循以下3个步骤**
#### 1.3.1 创建一个纹理对象并加载纹理数据
##### (1) 在OpenGL中使用纹理的第一步是创建纹理对象，然后将对象绑定到环境的纹理单元
```
glGenTextures(1, &m_textureObj);
glBindTexture(m_textureTarget, m_textureObj);
```
正如之前我们学到的创建顶点缓冲区和索引缓冲区用到的GLUT的函数glGenBuffers()，我们创建一个纹理对象需呀用到函数glGenTextures(),函数返回一个没有用到的整数(对象编号)赋值到m_textureObj

并且我们要绑定当前的纹理对象，就像使用glBindBuffer()绑定顶点缓冲对象和索引缓冲对象一样，在对纹理对象进行操作需要先绑定当前的纹理对象
    
##### (2) 创建纹理对象并且绑定了当前的纹理对象，我们就需要把我们的贴图加载到我们的纹理数据当中

然而我们的贴图在硬盘中存放，我们OpenGL应用程序无法直接从硬盘中加载纹理数据，所以需要我们先把硬盘中的贴图读取到内存里，再从内存中加载纹理数据，示意图如下：

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Texture/image/4.png)

```
unsigned char *data = stbi_load(m_fileName.c_str(),&width, &height, &nrChannels, 0);
...
glTexImage2D(m_textureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
```
[这里我们使用github上一个开源库——stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h)来读取存在硬盘里的图像数据

glTexImage2D()是根据指定的参数生成一个2D纹理

```
glTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
```
- target 指定目标纹理，这个值必须是GL_TEXTURE_2D。
- level 执行细节级别。0是最基本的图像级别，n表示第N级贴图细化级别。
- internalformat 指定纹理中的颜色组件。可选的值有GL_ALPHA,GL_RGB, GL_RGBA, GL_LUMINANCE,   GL_LUMINANCE_ALPHA 等几种。
- width 指定纹理图像的宽度，必须是2的n次方。纹理图片至少要支持64个材质元素的宽度
- height 指定纹理图像的高度，必须是2的m次方。纹理图片至少要支持64个材质元素的高度
- border 指定边框的宽度。**必须为0。**
- format 像素数据的颜色格式, 不需要和internalformatt取值必须相同。可选的值参考internalformat。
- type 指定像素数据的数据类型。可以使用的值有GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1。
- pixels 指定内存中指向图像数据的指针

#### 1.3.2 为顶点数据增加纹理坐标
我们在主程序main.cpp中为顶点数据增加纹理坐标，首先自定义一个数据结构，用来存放三维位置信息(一个长度为3的float类型数组)和二维纹理坐标(长度为2的数组)：
```
struct Vertex
{
	Vector3f m_pos;
	Vector2f m_tex;

	Vertex() {}

	Vertex(Vector3f pos, Vector2f tex)
	{
		CopyVector3(m_pos, pos);
		CopyVector2(m_tex, tex);
	}
};
```
随后我们创建顶点缓冲器时不仅包含我们的位置信息，还要包含纹理坐标信息
```
static void CreateVertexBuffer()
{
	Vector3f Vertices3f[4];
	Vector2f Vertices2f[4];
	LoadVector3(Vertices3f[0], -1.0f, -1.0f, 0.5773f);	LoadVector2(Vertices2f[0], 0.0f, 0.0f);
	LoadVector3(Vertices3f[1], 0.0f, -1.0f, -1.15475f); LoadVector2(Vertices2f[1], 0.5f, 0.0f);
	LoadVector3(Vertices3f[2], 1.0f, -1.0f, 0.5773f);	LoadVector2(Vertices2f[2], 1.0f, 0.0f);
	LoadVector3(Vertices3f[3], 0.0f, 1.0f, 0.0f);		LoadVector2(Vertices2f[3], 0.5f, 1.0f);
	Vertex Vertices[4] = { Vertex(Vertices3f[0],Vertices2f[0]),
		Vertex(Vertices3f[1],Vertices2f[1]),
		Vertex(Vertices3f[2],Vertices2f[2]),
		Vertex(Vertices3f[3],Vertices2f[3]) };

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}
```


#### 1.3.3 在着色器中使用纹理坐标从纹理中取样得到像素颜色
##### （1）首先我们把纹理坐标传送到着色器当中
在渲染函数Render()中完成纹理坐标到着色器的传送
```
    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(Vector3f));
```
启动一个顶点属性1用于纹理坐标，并且在glVertexAttribPointer()函数的参数设置偏移量为Vector3f,因为每个顶点坐标之后存放纹理坐标存，偏移量即为顶点坐标

##### （2）在顶点着色器中取得纹理坐标并传递到片元着色器
shader.vs:

```
#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

uniform mat4 gWVP;

out vec2 TexCoord0;

void main()
{
	gl_Position= gWVP * vec4(Position,1.0);
	TexCoord0=TexCoord;
}
```
可以看到同获取位置坐标一样，这里layout参数修饰符里用location=1来获取位置坐标到TexCoord，这里跟刚才glEnableVertexAttribArray(1)对应

##### （3）片元着色器中采样获取像素颜色
shader.fs:
```
#version 330

in vec2 TexCoord0;

out vec4 FragColor;

uniform sampler2D gSampler;
void main()
{
	FragColor=texture2D(gSampler,TexCoord0.st);
}
```
- 通过内部函数texture2D()对纹理进行取样，第一参数是取样器Uniform变量，这里的0与纹理单元(GL_TEXTURE0)对应，第二个参数是纹理坐标。
- 取样器的一致变量由主程序中传递过来

```
glUniform1i(gSampler, 0);
```
- 第二个坐标是顶点着色器传递过来的，这里声明一个输入变量TexCoord0来使用

## 二、增加纹理类
我们创建了一个Texture类，用来封装对的纹理操作。

opengl_texture.h:

```
#ifndef __OPENGL_TEXTURE_H
#define __OPENGL_TEXTURE_H
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture
{
public:
	Texture(GLenum TextureTarget, const std::string& FileName)
	{
		m_textureTarget = TextureTarget;
		m_fileName = FileName;
	}
	bool Load()
	{
		int width, height, nrChannels;
		unsigned char *data = stbi_load(m_fileName.c_str(),&width, &height, &nrChannels, 0);
		glGenTextures(1, &m_textureObj);
		glBindTexture(m_textureTarget, m_textureObj);
		glTexImage2D(m_textureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(m_textureTarget, 0);

		return true;
	}
	void Bind(GLenum TextureUnit)
	{
		glActiveTexture(TextureUnit);
		glBindTexture(m_textureTarget, m_textureObj);
	}
private:
	std::string m_fileName;
	GLenum m_textureTarget;
	GLuint m_textureObj;
};
#endif
```
- 可以看到纹理类中三个私有变量分别代表贴图文件名(m_fileName),纹理类型(m_textureTarget),纹理对象(m_textureObj)
- 类的构造方法接收两个参数，纹理类型和贴图文件名都直接赋值给内部变量
- 加载方法Load()在上面已经讲到，只有glTexParameterf()函数没有提到
```
glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```
这两个句用来设置纹理滤波方式。

由于纹理坐标不依赖于分辨率，且可以是任何浮点数，而纹理贴图中纹素的坐标以整数定义，如果纹理坐标映射到纹素上坐标为(152.34,745.14)，就需要用到滤波取一个颜色。

纹理滤波方式一般有两种：GL_NEAREST(邻近滤波,默认滤波方式)和GL_LINEAR(线性滤波)

**邻近滤波相当于舍去小数，把(152.34,745.14)变为(152,745),线性滤波选择周围纹素2x2的4个坐标 ( (152,745), (153,745), (152,744) 和 (153,744) ) 并根据他们的颜色做线性插值**


![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Texture/image/5.png)

纹理过滤可以指定放大和缩小操作，我们这里设置放大和缩小操作都是线性滤波方式

- Bind()函数用来把激活一个纹理单元并把纹理对象绑定到上面
```
glActiveTexture(TextureUnit);
```
glActiveTexture()函数用来激活纹理单元，这里参数使用纹理单元的枚举（GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2 等等）

比如我们在渲染主循环函数Render()中使用了

```
pTexture->Bind(GL_TEXTURE0)
```
我们在向着色器传递采样器Uniform变量时就传递的是0
```
glUniform1i(gSampler, 0);
```
## 三、其他代码
opengl_math.h, opengl_camera.h,opengl_pipeline.h, opengl_camera.cpp, opengl_pipeline.cpp都没有改变，因为代码量太大就不贴出来了，可以到github上下载来看

main.cpp：

```
#include <stdio.h>
#include <string>
#include <gl/glew.h>]
#include <gl/freeglut.h>
#include <assert.h>
#include <fstream>

#include "opengl_math.h"
#include "opengl_pipeline.h"
#include "opengl_texture.h"

#define Window_Width 1024
#define Window_Height 768
using namespace std;

GLuint VBO,IBO;
GLuint gWVPLocation;
GLuint gSampler;
Camera* GameCamera = NULL;
Texture* pTexture = NULL;
PersProjInfo gPersProjInfo;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

struct Vertex
{
	Vector3f m_pos;
	Vector2f m_tex;

	Vertex() {}

	Vertex(Vector3f pos, Vector2f tex)
	{
		CopyVector3(m_pos, pos);
		CopyVector2(m_tex, tex);
	}
};

static void Render()
{
	GameCamera->OnRender();
	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;
	Scale += 0.1f;

	Pipeline p;
	p.Rotate(0.0f, Scale, 0.0f);
	p.WorldPos(0.0f, 0.0f, 3.0f);
	p.SetCamera(*GameCamera);
	p.SetPerspectiveProj(gPersProjInfo);

	glUniformMatrix4fv(gWVPLocation, 1, GL_FALSE, (const GLfloat*)p.GetWVPTrans());

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(Vector3f));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	pTexture->Bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glutSwapBuffers();
}
static void Keyboard(int key, int x, int y)
{
	GameCamera->OnKeyboard(key);
}
static void Mouse(int x, int y)
{
	GameCamera->OnMouse(x, y);
}
static void InitializeCallbacks()
{
	glutDisplayFunc(Render);
	glutIdleFunc(Render);

	glutPassiveMotionFunc(Mouse);
	glutSpecialFunc(Keyboard);
}
static void CreateVertexBuffer()
{
	Vector3f Vertices3f[4];
	Vector2f Vertices2f[4];
	LoadVector3(Vertices3f[0], -1.0f, -1.0f, 0.5773f);	LoadVector2(Vertices2f[0], 0.0f, 0.0f);
	LoadVector3(Vertices3f[1], 0.0f, -1.0f, -1.15475f); LoadVector2(Vertices2f[1], 0.5f, 0.0f);
	LoadVector3(Vertices3f[2], 1.0f, -1.0f, 0.5773f);	LoadVector2(Vertices2f[2], 1.0f, 0.0f);
	LoadVector3(Vertices3f[3], 0.0f, 1.0f, 0.0f);		LoadVector2(Vertices2f[3], 0.5f, 1.0f);
	Vertex Vertices[4] = { Vertex(Vertices3f[0],Vertices2f[0]),
		Vertex(Vertices3f[1],Vertices2f[1]),
		Vertex(Vertices3f[2],Vertices2f[2]),
		Vertex(Vertices3f[3],Vertices2f[3]) };

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}
static void CreateIndexBuffer()
{
	unsigned int Indices[] =
	{
		0,3,1,
		1,3,2,
		2,3,0,
		0,1,2
	};

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}
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
		fprintf(stderr, "%s : %d unable to read file: %s", __FILE__, __LINE__, FileName);
		system("pause");
		exit(1);
	}
	return ret;
}
static void Addshader(GLuint ShaderProgram,const char* ShaderText,GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj) {
		fprintf(stderr, "Error creating shder object");
		system("pause");
		exit(1);
	}

	const GLchar* p[1];
	p[0] = ShaderText;
	GLint Length[1];
	Length[0] = strlen(ShaderText);
	glShaderSource(ShaderObj, 1, p, Length);
	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj,sizeof(InfoLog),NULL,InfoLog);
		fprintf(stderr, "Error compiling shader object: '%s'", InfoLog);
		system("pause");
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
	
}
static void CompileShader()
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

	Addshader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	Addshader(ShaderProgram,fs.c_str(),GL_FRAGMENT_SHADER);

	GLchar ErrorLog[1024] = { 0 };
	GLint Success;

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'", ErrorLog);
		system("pause");
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Valied to shader program:'%s'", ErrorLog);
		system("pause");
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
	assert(gWVPLocation != 0xFFFFFFFF);
	gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
	assert(gSampler != 0xFFFFFFFF);
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(Window_Width, Window_Height);
	glutCreateWindow("Texture");
	glutGameModeString("1280x1024@32");
	InitializeCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "unable to init glew");
		system("pause");
		exit(1);
	}

	printf("GL version : '%s'", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	CreateVertexBuffer();
	CreateIndexBuffer();

	CompileShader();

	glUniform1i(gSampler, 0);

	pTexture = new Texture(GL_TEXTURE_2D, "test.png");

	if (!pTexture->Load()) {
		return 1;
	}

	gPersProjInfo.FOV = 60.0f;
	gPersProjInfo.Height = Window_Height;
	gPersProjInfo.Width = Window_Width;
	gPersProjInfo.zNear = 1.0f;
	gPersProjInfo.zFar = 100.0f;

	Vector3f CameraPos, CameraTarget, CameraUp;
	LoadVector3(CameraPos, 0.0f, 0.0f, -4.0f);
	LoadVector3(CameraTarget, 0.0f, 0.0f, 1.0f);
	LoadVector3(CameraUp, 0.0f, 1.0f, 0.0f);
	GameCamera = new Camera(Window_Width, Window_Height, CameraPos, CameraTarget, CameraUp);

	glutMainLoop();

	return 0;
}
```
这里的相较于上一节的改变，大部分已经在上面讲到了，有几处还没提到：

```
pTexture = new Texture(GL_TEXTURE_2D, "test.png");

if (!pTexture->Load()) {
		return 1;
	}
```
这里创建纹理对象，并加载我们在同一工程目录下的test.png贴图文件


```
glFrontFace(GL_CW);	glCullFace(GL_BACK);	glEnable(GL_CULL_FACE);
```
- 这三个函数只是为了让效果看上去更好
- 这些函数开启面的剔除，在光栅化之前将一些不需要的三角形丢掉，因为在自然世界中物体的背面我们是看不到的
- glFrontFace()用来设置顺时针多边形为正面，GL_CW(顺时针为正面)，GL_CCW(逆时针为正面)

    索引里三角形顺序都是顺时针所以需要设置顺时针为正面，不然默认逆时针为正面
- glCullFace()用来设置GPU去除三角形的背面，即物体的内表面不需要渲染只渲染外表面
- glEnable(GL_CULL_FACE)用来开启剔除面

## 四、运行结果
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Texture/image/6.png)
