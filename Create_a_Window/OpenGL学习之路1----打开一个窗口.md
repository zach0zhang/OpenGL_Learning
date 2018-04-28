根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

OpenGL（全写Open Graphics Library）是指定义了一个跨编程语言、跨平台的编程接口规格的专业的图形程序接口。它用于三维图像（二维的亦可），是一个功能强大，调用方便的底层图形库。

先贴出源码：

main.c:

```
#include <gl/freeglut.h>

void RenderSceneCB() {
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Create a Window");

	glutDisplayFunc(RenderSceneCB);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glutMainLoop();

	return 0;
}
```


```
glutInit(&argc, argv);
```
根据函数名就能看出来是初始化GLUT


```
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
```
设置显示模式

GLUT_DOUBLE意思是两个缓冲区，显示一个缓冲区的时候，在另一个缓冲区绘制(两个缓冲区循环往复显示---绘制)

GLUT_RGBA显示的颜色模式,RGBA代表Red（红色） Green（绿色） Blue（蓝色）和 Alpha(透明度)


```
glutInitWindowSize(1024, 768);
glutInitWindowPosition(10, 10);
glutCreateWindow("Create a Window");
```
设置窗口大小为1024*768，窗口位置以屏幕左上角为原点的(10,10)处，创建窗口并且命名为“Create a Window"

```
glutDisplayFunc(RenderSceneCB);
```
设置显示回调函数为RenderSceneCB,因为程序跑在一个窗口系统里，所以大多数工作通过回调函数完成，这里的RenderSceneCB即是渲染需要的回调函数


```
glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
```
设置好"清除颜色"的颜色，从左到右的参数分别是红、绿、蓝、透明度，(0.0f,0.0f,0.0f,0.0f)即为黑


```
glutMainLoop();
```
这个函数表明开始内部循环操作，即进入 侦听窗口事件发生---->调用回调函数处理，本程序只有显示回调函数(RenderSceneCB)不停回调执行

```
void RenderSceneCB() {
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}
```
RenderSceneCB回调函数

首先使用之前glClearColor()设置好的清除颜色(GL_COLOR_BUFFER_BIT)清除当前窗口颜色，

再进行替换当前显示缓冲区，即和glutInitDisplayMode设置的GLUT_DOUBLE模式对应起来，循环显示两个缓冲区


**运行结果：**

![image](E:\笔记\Opengl\Create a Window\1.png)