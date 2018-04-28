根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

到目前为止，我们已经可以将一个物体以任意大小、任意角度和任意位置摆放在世界空间，相关博文链接：
[OpenGL学习之路6----平移，旋转和缩放变换](https://blog.csdn.net/zach_z/article/details/80072929)

[OpenGL学习之路9----混合变换](https://blog.csdn.net/zach_z/article/details/80086523)

并且可以将我们的相机任意摆放在世界空间，相关博文链接：
[OpenGL学习之路11----相机空间](https://blog.csdn.net/zach_z/article/details/80102219)

上一节通过键盘上的方向键，实现了相机在世界空间里位置的移动，相关链接：
[OpenGL学习之路12----相机控制(键盘操作)](https://blog.csdn.net/zach_z/article/details/80111542)

**这次我们要通过移动鼠标，实现改变相机的目标方向**

## 一、背景知识
### 1.1 水平倾角和垂直倾角
小时候玩过的一款经典游戏《抢滩登陆战》，我们无法移动，但是可以通过鼠标控制我们的攻击方向，正如下图中的炮：
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Mouse/image/1.png)
- 假设它在我们的世界坐标中，它可以绕向量(0,1,0)旋转360°，旋转角度称为“水平倾角”，这个(0,1,0)向量叫做垂直轴线(比如转到身后去打背后的敌人)
- 也可以上下倾斜旋转，绕着与地面平行的向量，这个倾角叫做“垂直倾角”，这个向量叫做“水平轴线”
### 1.2 水平倾角计算target向量
我们从y轴竖直向下看y轴负方向：
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Mouse/image/2.png)
可以看到根据水平倾角α可以计算target(目标方向向量)

```
target.x = cosα 
target.y = 0
target.z = sinα
```
### 1.3 万向锁现象
**根据垂直倾角来计算target向量，需要考虑水平旋转后水平轴线产生变化。 虽然水平轴线可以由垂直轴线和target向量进行叉积后得到，但是会有可能产生一个叫万向锁的现象。**

首先我们明确一点假如一个物体初始 旋转(0,0,0)，第一次旋转10度到(0,10,0),当在这个位置时想让它旋转到(0,20,0),**它不是从(0,10,0)旋转10度到(0,20,0)而是先回到(0,0,0)再旋转到(0,20,0)**

如果初始在(0,0,0)要旋转到(10,20,30),我们规定先旋转Z轴，再旋转Y轴，再旋转X轴，即
```
(0,0,0) -> (0,0,30) -> (0,20,30) -> (10,20,30)
```

万向锁的概念很好理解：
- 如果你手上有手机，请将手机水平平放置,如下图所示：
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Mouse/image/3.png)
- 此时给绕Z轴转一个角度，**可以发现X轴和Y轴随着手机绕Z轴转动而改变了**，再绕Y轴旋转一个角度，再绕X轴旋转一个角度，我们就可以到让手机指向三维空间任意一个方向
- **但是如果绕Z轴旋转一定角度后，在绕Y轴旋转的时候不小心旋转了90°，这时候怎么绕X轴手机都不会立起来，好像被锁在了桌面上**
- 然而绕y轴如果不等于90°，就可以使手机旋转指向三维空间任意一个方向

这里产生万向锁的原因就在于第二次旋转了90°，使得第二次旋转后的x轴和没旋转开始时候的Z轴重合了，所以旋转X轴就相当于重复第一次旋转而没进行“抬高”的动作

### 1.4 四元数解决万向锁问题
定义一个四元数Q为：

```math
Q=xi+yj+zk+w
```
i,j,k都是负数且满足

```math
 i^2=j^2=k^2=ijk=-1
```
四元数Q的共轭量定义为：

```math
Q^{-1}=-xi-yj-zk+w
```
我们可以使用一个四元数来描述如何绕任意向量旋转一个向量，具体数学证明可以自行研究，这里只给出公式。
V向量旋转a度，得到的W向量方法为：
```math
W = QVQ^{-1}
```
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Mouse/image/4.png)

### 1.5 一个向量绕另一个向量旋转的代码表示
```
//向量v旋转a度：
//				W = Q * v * Q^(-1)
//	Q = ( Vx * sin(a/2), Vy * sin(a/2), Vz * sin(a/2), cos(a/2) )
//	Q^(-1) = ( -Vx * sin(a/2), -Vy * sin(a/2), -Vz * sin(a/2), cos(a/2) )
//v:要旋转的向量，Angle：旋转角度，Axe：围绕的向量
inline void RotateVector3(Vector3f v,float Angle,Vector3f Axe)
{
	const float SinHalfAngle = sinf(DegToRad(Angle / 2));
	const float CosHalfAngle = cosf(DegToRad(Angle / 2));

	const float Rx = Axe[0] * SinHalfAngle;
	const float Ry = Axe[1]* SinHalfAngle;
	const float Rz = Axe[2] * SinHalfAngle;
	const float Rw = CosHalfAngle;
	
	Vector4f RotationQ;//四元数
	LoadVector4(RotationQ,Rx, Ry, Rz, Rw);
	Vector4f ConjugateQ;
	LoadVector4(ConjugateQ, -Rx, -Ry, -Rz, Rw);
	//  ConjugateQ.Normalize();
	Vector4f W,V;
	MultiplyVector4and3(W, RotationQ, v); 
	MultiplyVector4and4(V, W, ConjugateQ);
	v[0] = V[0];
	v[1] = V[1]; 
	v[2] = V[2];
}
```
- 我们在3d数学库(opengl_math.h)中添加向量绕另一个向量旋转的内联函数，按照我们的公式W=QVQ^-1最终得到旋转后向量
- 四元数用一个长度为4的浮点型数组表示

## 二、相机类的扩充
opengl_camera.h:
```
#ifndef __OPENGL_CAMERA_H
#define __OPENGL_CAMERA_H
#include "opengl_math.h"
#include <gl/freeglut_std.h>

class Camera
{
private:
	Vector3f m_pos;
	Vector3f m_target;
	Vector3f m_up;

	float m_AngleH;
	float m_AngleV;

	bool m_OnUpperEdge;
	bool m_OnLowerEdge;
	bool m_OnLeftEdge;
	bool m_OnRightEdge;

	Vector2i m_mousePos;

	int m_windowWidth;
	int m_windowHeight;
public:
	Camera(int Window_Width, int Window_Height)
	{
		m_windowWidth = Window_Width;
		m_windowHeight = Window_Height;
		LoadVector3(m_pos,0.0f, 0.0f, 0.0f);
		LoadVector3(m_target,0.0f, 0.0f, 1.0f);
		LoadVector3(m_target,0.0f, 1.0f, 0.0f);
	}
	Camera(int Window_Width, int Window_Height, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
	{
		m_windowWidth = Window_Width;
		m_windowHeight = Window_Height;
		CopyVector3(m_pos, Pos);
		CopyVector3(m_target, Target);
		NormalizeVector3(m_target);
		CopyVector3(m_up, Up);
		NormalizeVector3(m_up);

		Init();
	}
	void Init();
	bool OnKeyboard(int key);
	void OnMouse(int x, int y);
	void Update();
	void OnRender();
	const Vector3f& GetPos()
	{
		return m_pos;
	}
	const Vector3f& GetTarget()
	{
		return m_target;
	}
	const Vector3f& GetUp()
	{
		return m_up;
	}
};

#endif
```

### 2.1 相机初始化
可以看到我们的相机类里增加了水平倾角和垂直倾角两个私有变量
```
	float m_AngleH;
	float m_AngleV;
```
- 这两个角度的值在我们移动鼠标时，会被不断更新。
- 这两个角度的值需要在相机类构造方法中被初始化一个角度
- 可以看到构造函数中多出来一个Init()方法的引用

在opengl_camera.cpp中实现了Init()方法：
```
void Camera::Init()
{
	Vector3f HTarget;
	LoadVector3(HTarget, m_target[0], 0.0, m_target[2]);
	NormalizeVector3(HTarget);

	if (HTarget[2] >= 0.0f)
	{
		if (HTarget[0] >= 0.0f)
		{
			m_AngleH = 360.0f - RadToDeg(asin(HTarget[2]));
		}
		else
		{
			m_AngleH = 180.0f + RadToDeg(asin(HTarget[2]));
		}
	}
	else
	{
		if (HTarget[0] >= 0.0f)
		{
			m_AngleH = RadToDeg(asin(-HTarget[2]));
		}
		else
		{
			m_AngleH = 90.0f + RadToDeg(asin(-HTarget[2]));
		}
	}

	m_AngleV = -RadToDeg(asin(m_target[1]));

	m_OnUpperEdge = false;
	m_OnLowerEdge = false;
	m_OnLeftEdge = false;
	m_OnRightEdge = false;
	m_mousePos[0] = m_windowWidth / 2;
	m_mousePos[1] = m_windowHeight / 2;

	glutWarpPointer(m_mousePos[0], m_mousePos[1]);

}
```
- 可以看到根据target向量的x和z分量来确定m_AngleH(水平倾角)
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Mouse/image/5.png)
对z分量反sin之后就可以得到α角，但是我们四元数控制的是顺时针旋转，所以**可以根据target向量的x和z分量的正负来确定是在第几象限**，再判断最终的水平倾角

    例如上图，计算出阿尔法角度后，因为x和z都为正所以是第一向量，所以用360°-α得到最终角度
- 我们规定垂直倾角最大为90°，可以通过target的y分量来得到m_AngleV(垂直倾角)
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Mouse/image/6.png)
可以得出垂直倾角即为对y分量反sin后得到的角度再取负(因为四元数旋转是顺时针)
- 同时初始化的时候设置了其他相机类内新加的私有变量的值
```
    m_OnUpperEdge = false;
	m_OnLowerEdge = false;
	m_OnLeftEdge = false;
	m_OnRightEdge = false;
	m_mousePos[0] = m_windowWidth / 2;
	m_mousePos[1] = m_windowHeight / 2;

	glutWarpPointer(m_mousePos[0], m_mousePos[1]);
```
分别把触及上下左右四个边界的布尔变量设为flase,**并把鼠标移动到屏幕中心**

### 2.2 鼠标事件函数
相机类中的方法OnMouse()用来处理鼠标事件，接收当前鼠标的位置x和y作为参数，在opengl_camera.cpp中实现：
```
void Camera::OnMouse(int x, int y)
{
	const int DeltaX = x - m_mousePos[0];
	const int DeltaY = y - m_mousePos[1];

	m_mousePos[0] = x;
	m_mousePos[1] = y;

	m_AngleH += (float)DeltaX / 20.0f;
	m_AngleV += (float)DeltaY / 20.0f;

	if (DeltaX == 0) {
		if (x <= MARGIN) {
			m_OnLeftEdge = true;
		}
		else if (x >= (m_windowWidth - MARGIN)) {
			m_OnRightEdge = true;
		}
	}
	else {
		m_OnLeftEdge = false;
		m_OnRightEdge = false;
	}
	if (DeltaY == 0) {
		if (y <= MARGIN) {
			m_OnUpperEdge = true;
		}
		else if (y >= (m_windowHeight - MARGIN)) {
			m_OnLowerEdge = true;
		}
	}
	else {
		m_OnUpperEdge = false;
		m_OnLowerEdge = false;
	}
	Update();
}
```
- 可以看到先计算当前鼠标的位置和上一次鼠标位置的偏移，再除上一个缩放比例(如果不进行缩放移动鼠标会改变很大的角度，不太好观察改变，不同电脑可以调整不同的值)
- 根据偏移位置改变水平偏角和垂直偏角
- 再判断是否到达边界，这里边界的界定是可以通过设置MAGIN的值来决定的，这里10像素以内就判定到达边界
- 最后调用Update()根据改变后的水平倾角和垂直倾角重新计算target向量和up向量

### 2.3 边缘处理方法
如果在鼠标处理函数中判定到达了边界，则要对相应的边界布尔变量置位，而且也需要一个处理到达边界的方法，在opengl_camera.cpp中也已经实现：

```
void Camera::OnRender()
{
	bool ShouldUpdate = false;

	if (m_OnLeftEdge) {
		m_AngleH -= EDGE_STEP;
		ShouldUpdate = true;
	}
	else if (m_OnRightEdge) {
		m_AngleH += EDGE_STEP;
		ShouldUpdate = true;
	}

	if (m_OnUpperEdge) {
		if (m_AngleV > -90.0f) {
			m_AngleV -= EDGE_STEP;
			ShouldUpdate = true;
		}
	}
	else if (m_OnLowerEdge) {
		if (m_AngleV < 90.0f) {
			m_AngleV += EDGE_STEP;
			ShouldUpdate = true;
		}
	}
	if (ShouldUpdate) {
		Update();
	}
}
```
- 如果到达左边界则对水平倾角减去0.5度，即是逆时针绕着垂直轴线转。其他边界同理
- 最后调用Update()根据改变后的水平倾角和垂直倾角重新计算target向量和up向量

### 2.4 重新计算target和up向量
在opengl_camera.cpp里实现了Update()方法：

```
void Camera::Update()
{
	Vector3f Vaxis;
	LoadVector3(Vaxis, 0.0f, 1.0f, 0.0f);
	// Rotate the view vector by the horizontal angle around the vertical axis
	Vector3f View;
	LoadVector3(View, 1.0f, 0.0f, 0.0f);
	RotateVector3(View, m_AngleH, Vaxis);
	NormalizeVector3(View);

	// Rotate the view vector by the vertical angle around the horizontal axis
	Vector3f Haxis;
	CrossProduct3(Haxis, Vaxis, View);
	NormalizeVector3(Haxis);

	RotateVector3(View, m_AngleV, Haxis);

	CopyVector3(m_target, View);
	NormalizeVector3(m_target);

	CrossProduct3(m_up, m_target, Haxis);
	NormalizeVector3(m_up);
}
```
- 首先直接假定自己相机朝向View向量看向X轴正方向
- 再把View绕着y轴旋转水平倾角
- 把y轴和View叉积得到向量Haxis,即是旋转水平倾角后的X轴
- 再把View绕着目前相机第一次旋转后的X轴(Haxis)旋转垂直倾角
- 此时View即为m_target(即是相机朝向向量)
- m_target和Haxis(现在相机的X轴，也就是相机的y轴)叉积后得到相机头顶方向向量

## 三、主程序中的相关改变
### 3.1 注册鼠标移动回调函数
同之前Render()函数被注册为显示函数，Keyboard()函数被注册为键盘相应函数，相应事件到来时，会执行相应的回调函数

**我们处理鼠标移动也要用glutPassiveMotionFunc()函数来把我们的鼠标移动处理函数注册成为鼠标没有按下时移动的回调函数：**

```
...
static void Mouse(int x, int y)
{
	GameCamera->OnMouse(x, y);
}
static void InitializeGlutCallbacks()
{
	glutDisplayFunc(Render);
	glutIdleFunc(Render);

	glutPassiveMotionFunc(Mouse);
	glutSpecialFunc(Keyboard);

}
...
```
- 函数的参数是当前鼠标的位置x,y
- 我们用的是鼠标没有按下去时移动鼠标的事件处理注册函数，而鼠标按下去移动鼠标需要用glutMotionFunc()

### 3.2 设置游戏模式
在main.cpp中我们设置到游戏模式
```
glutGameModeString("1920x1200:32"); 
glutEnterGameMode();
```
- glutGameModeString()函数用来设置全屏模式屏幕像素为1920*1200，每个像素的比特数是32bit(32位真彩色)
- glutEnterGameMode()函数用来进入我们设置好的游戏模式
- **这么做我们可以全屏显示，而且更好的进行鼠标移动处理中的边缘判断**

### 3.3 主渲染中调用相机类的OnRender()

```
static void Render()
{
	GameCamera->OnRender();
	...
}
```
一开始Render就调用相机类的方法OnRender(),用来处理到达边缘事件时相机的旋转操作。

## 四、运行结果
![image](https://github.com/zach0zhang/OpenGL_Learning/blob/master/Camera_Mouse/image/7.png)
可以通过鼠标调整相机的位置，到达左右边缘后继续旋转水平倾角，可以水平旋转360°回到原点
