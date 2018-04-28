根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

## 归一化坐标
我们在屏幕上绘制图形的时候，是在一个归一化的空间里，即为下图

![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/First_Triangle/image/1.png)

可以看到绘制窗口左下角坐标为(-1.0,-1.0)到右上角坐标为(1.0,1.0)

## 代码解释
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

```
#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "opengl_math.h"

GLuint VBO;

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

static void CreateVertexBuffer()
{
	Vector3f Vertices[3];

	LoadVector3(Vertices[0], -0.5f, -0.5f, 0.0f);
	LoadVector3(Vertices[1], 0.5f, -0.5f, 0.0f);
	LoadVector3(Vertices[2], 0.0f, 0.5f, 0.0f);

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}
int main(int argc, char ** argv) {

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(480, 320);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("First Triangle");

	glutDisplayFunc(Render);

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();

	glutMainLoop();

	return 0;
}


```
这一节的代码跟上一节只有一点区别，这里只说区别。


```
Vector3f Vertices[3];

	LoadVector3(Vertices[0], -0.5f, -0.5f, 0.0f);
	LoadVector3(Vertices[1], 0.5f, -0.5f, 0.0f);
	LoadVector3(Vertices[2], 0.0f, 0.5f, 0.0f);
```
这里定义了包含三个顶点的数组，并且给分别赋了值

```
glDrawArrays(GL_TRIANGLES, 0, 3); 
```
绘制几何图形的第一个参数变成了绘制三角形，定点数变为了3个

## 运行结果：
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/First_Triangle/image/2.png)
