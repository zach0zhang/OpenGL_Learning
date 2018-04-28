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