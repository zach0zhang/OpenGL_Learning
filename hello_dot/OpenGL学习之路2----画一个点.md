根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程
## 一、简述
这一节比上一节《打开一个窗口》，多了需要使用glew库和3d数学库

glew是OpenGL的扩展库，用于帮助C/C++开发者初始化扩展（OpenGL扩展功能）并书写可移植的应用程序。这个可以通过上网搜教程自行安装引用。

3d函数库可以用网上的，也可以用源教程里的。这里我是根据需要开始自己写了一个3d数学的头文件，很好理解，也更利于学习计算机图形学，根据每章教程需要用的不同数学表达，逐渐增加我的3d数学文件。

## 二、代码解释

opengl_math.h:

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
main.c:

```
#include <stdio.h>
#include <gl/glew.h>     // GLEW扩展库,这个引用要放在freeglut.h前
#include <gl/freeglut.h> // freeGLUT图形库
#include "opengl_math.h" 

GLuint VBO;

static void RenderScenceCB() {
	glClear(GL_COLOR_BUFFER_BIT);

	// 绑定GL_ARRAY_BUFFER缓冲器
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// 告诉管线怎样解析bufer中的数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// 开始绘制几何图形(绘制一个点)
	glDrawArrays(GL_POINTS, 0, 1);

	// 交换前后缓存
	glutSwapBuffers();
}
static void CreateVertexBuffer()
{
	// 创建含有一个顶点的顶点数组
	Vector3f Vertices[1];
	// 将点置于屏幕中央
	LoadVector3(Vertices[0], 0.0f, 0.0f, 0.0f);

	// 创建缓冲器
	glGenBuffers(1, &VBO);
	// 绑定GL_ARRAY_BUFFER缓冲器
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// 绑定顶点数据
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

int main(int argc, char ** argv) {

	// 初始化GLUT
	glutInit(&argc, argv);

	// 显示模式：双缓冲、RGBA
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	// 窗口设置
	glutInitWindowSize(480, 320);      // 窗口尺寸
	glutInitWindowPosition(100, 100);  // 窗口位置
	glutCreateWindow("Hello dot");   // 窗口标题

									   // 开始渲染
	glutDisplayFunc(RenderScenceCB);

	// 检查GLEW是否就绪，必须要在GLUT初始化之后！
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	// 缓存清空后的颜色值
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// 创建顶点缓冲器
	CreateVertexBuffer();

	// 开始GLUT的内部循环
	glutMainLoop();

	return 0;
}
```

### 2.1 3d数学库
```
typedef float   Vector3f[3];
```
首先是3d数学库，本次只用到了三维向量，我们使用一个长度为3的浮点数数组来表示一个三维向量

```
inline void LoadVector3(Vector3f v, const float x, const float y, const float z)
{
	v[0] = x; v[1] = y; v[2] = z;
}
```
### 2.2 创建顶点缓冲器
通过一个内联函数，可以给三维向量(长度为三的浮点数数组)赋值


```
Vector3f Vertices[1];
LoadVector3(Vertices[0], 0.0f, 0.0f, 0.0f);
```
CreateVertexBuffer()创建顶点缓冲器函数里，创建了一个只有一个顶点的顶点数组，再给那个顶点赋值为(0.0f,0.0f,0.0f)即在空间的位置是原点(这里会绘制在屏幕中心)


```

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
```
在创建顶点缓冲器(以后的教程还会创建索引缓冲期)的时候，这三个函数一般都连续出现。


```
void glGenBuffers(GLsizei n ,GLuint *buffers)
```
 
- 返回n个当前未使用的缓存对象名称，并保存到buffers数组中。返回到buffers中的名称不一定是连续的整型数据。如果n是负数，那么产生GL_INVALID_VALUE错误。
- 所以glGenBuffers(1, &VBO)是返回1个当前未使用的缓存对象的名称，并保存到VBO里
- VBO是一个全局GLuint(其实就是unsigned int)型的变量


```
void glBindBuffer(GLenum target,GLuint buffer)
```
- 这个函数的功能是指定当前激活的对象
- target是缓存对象的类型（GL_ARRAY_BUFFER,GL_ATOMIC_COUNTER_BUFFER,GL_ELEMENT_ARRAY_BUFFER等等...),这里使用GL_ARRAY_BUFFER代表绑定顶点缓冲区对象
- buffer设置的是要绑定的缓存对象的名称，这里用刚才创建的缓存对象VBO
- 此时VBO成为当前target中被激活的对象，现在程序小，只有一个缓存对象，以后程序大起来，每次对缓存对象进行操作时都需要先使用这个函数激活缓存对象

```
glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
```
创建了缓冲器，绑定了激活了它，现在就是把顶点的位置信息绑定在它上面,第一个参数是缓存对象类型，第二个参数是数据的长度，第三个参数是数据的地址，最后一个参数说明是静态数据，顶点数据一经初始化便不会改变

### 2.3 glew初始化

```
GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
```
在glut初始化后，检查glew是否就绪，如果glew初始化没什么问题，程序可以继续运行，否则退出

### 2.4 渲染函数

```
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, 1);
```
先是绑定缓冲对象到GL_ARRAY_BUFFER缓冲器上，之后对其操作

```
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
```
- 第一个参数是属性的索引，这里只有一组数据默认为0，如果开始使用shader着色器时，我们既要明确的设置着色器中的属性索引同时也要检索它
- 第二个参数指属性中元素的个数，这里是3个，我们长度为3的浮点数数组，同时也代表XYZ的坐标
- 第三个参数指元素的数据类型，这里时FLOAT
- 第四个参数指明我们是否想让我们的属性在被管线使用之前被单位化，这里数据不变所以设为0
- 第五个参数是偏移距离，我们通常不止存储位置信息，假如我们存储的数据是一组(X,Y,Z,R,G,B),而这一组RGB的偏移位置就是X+Y+Z的大小(以后用到的时候会理解的)

```
glDrawArrays(GL_POINTS, 0, 1);
```
- 最后调用这个函数绘制几何图形，第一个参数定义画点，即每一个顶点只表示一个点
- 第二个从拿书是绘制顶点的索引，因为就一个顶点数据，所以从最开始绘制，设置为0
- 第三个参数是绘制的顶点数，1个顶点所以设置为1

## 运行结果
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/hello_dot/image/1.png)
