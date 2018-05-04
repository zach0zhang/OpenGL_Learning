根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

上一节学习了怎么渲染环境光，相关博文：
[OpenGL学习之路15----环境光](https://blog.csdn.net/zach_z/article/details/80188764)

## 漫射光
之前我们学习到的环境光忽视光线的方向，如果只有环境光，整个场景是被均匀照亮的

漫射光跟环境光唯一的区别是漫射光依赖光线的方向，漫射光照射到物体上，被照射的一面要比背面亮

漫射光的强度，与光线入射的角度有关，如果光线垂直物体，则漫射光强最大，随着入射光线与法线夹角的增大，慢射光强逐渐减小

如图所示，随着当α为0°时，即垂直照射到物体表面，和法线成180°，漫射光强最大；而随着α增大，漫射光强逐渐减小：
![image](E:/笔记/Opengl/Diffuse_Lighting/1.png)

计算光的强度，使用光线和物体表面法线的夹角α的余弦值作为一个参数，下图中表示了四个不同角度照射到表面上的情况：
![image](E:/笔记/Opengl/Diffuse_Lighting/2.png)
- A的强度是最大的，因为α为0，余弦值最大为1
- B的强度是，cos(α)
- C从表面的一侧入射，此时光线和法线垂直，所以α角度为90，余弦值为0，即光线C对表面照亮没有任何效果
- D从表面的背面入射，α是一个钝角，余弦值为负，光线D也对物体表面没有照亮作用

**结构体表示漫反射：**

```
struct DirectionalLight
{
	Vector3f Color;
	float AmbientIntensity;
	Vector3f Direction;
	float DiffuseIntensity;
};
```
- 可以看到我们平行光结构体，在上一节环境光两个属性参数的基础上增加了两个代表着漫反射属性参数的变量
- Direction：代表光线入射的方向向量
- DiffuseIntensity：表示为漫反射的光线强度

## 计算顶点法线
多边形面上分布的任意法向量都是相同的，所以一个面取一个法向量即可代表平面法向量。 **但是这样会导致一个平面上光线强度一样，导致多边形几个面之间光线不平滑。**

这时引入“顶点法线”的概念：**顶点法线即是共用一个顶点的所有三角形法线的平均值**

我们在顶点着色器中根据顶点法线顶点计算出在世界空间里每个点的法向量的值，并传递到片元着色器中计算出每个像素法向量对应的光照强度

如下图，左边是两个平面各自的法向量，而右边是由顶点法线计算后得到每个点的法向量的样子：
![image](E:/笔记/Opengl/Diffuse_Lighting/3.png)

计算顶点法线的代码：

```
static void CalcNormals(const unsigned int* pIndices, unsigned int IndexCount, Vertex* pVertices, unsigned int VertexCount)
{
	for (unsigned int i = 0; i < IndexCount; i += 3) {
		unsigned int Index0 = pIndices[i];
		unsigned int Index1 = pIndices[i+1];
		unsigned int Index2 = pIndices[i+2];
		Vector3f v1, v2;
		SubVector3(v1, pVertices[Index1].m_pos, pVertices[Index0].m_pos);
		SubVector3(v2, pVertices[Index2].m_pos, pVertices[Index0].m_pos);
		Vector3f Normal;
		CrossProduct3(Normal, v1, v2);
		NormalizeVector3(Normal);

		AddVector3(pVertices[Index0].m_normal, pVertices[Index0].m_normal, Normal);
		AddVector3(pVertices[Index1].m_normal, pVertices[Index1].m_normal, Normal);
		AddVector3(pVertices[Index2].m_normal, pVertices[Index2].m_normal, Normal);
	}
	for (unsigned int i = 0; i < VertexCount; i++) {
		NormalizeVector3(pVertices[i].m_normal);
	}
}
```
- 先根据索引取出每个三角形的三个顶点计算其法向量
- 每个三角形法向量都是从第一个顶点到其他两个顶点两条边向量进行差积
- 差积之后得到的三角形法向量进行单位化
- 把单位化后的三角形法向量与顶点法向量累加
- 最后再把所有顶点的顶点法向量单位化

## 漫射光的实现
- 扩充顶点结构体，假如顶点法向量变量

```
struct Vertex
{
	Vector3f m_pos;
	Vector2f m_tex;
	Vector3f m_normal;

	Vertex() {}

	Vertex(Vector3f pos, Vector2f tex)
	{
		CopyVector3(m_pos, pos);
		CopyVector2(m_tex, tex);
		LoadVector3(m_normal, 0.0f, 0.0f, 0.0f);
	}
};
```
- 扩充平行光结构体数组，假如漫反射变量参数，射入方向向量和光强

```
struct DirectionalLight
{
	Vector3f Color;
	float AmbientIntensity;
	Vector3f Direction;
	float DiffuseIntensity;
};
```
- 声明一个平行光结构体变量，并赋予初值，光线颜色为白色(1.0f,1.0f,1.0f)，环境光强为0.1f，漫反射光强为0.75f，漫反射光方向为(1.0f,0.0,0.0)即射向X轴正方向

```
struct DirectionalLight m_directionalLight;

LoadVector3(m_directionalLight.Color,1.0f, 1.0f, 1.0f);
m_directionalLight.AmbientIntensity = 0.1f;
m_directionalLight.DiffuseIntensity = 0.75f;
LoadVector3(m_directionalLight.Direction,1.0f, 0.0, 0.0);
```

- 计算出顶点法向量的值，放在各个顶点结构体中，存入顶点缓冲区，CalcNormals()函数用来计算每个顶点的顶点法向量

```
CalcNormals(Indices, sizeof(Indices)/sizeof(Indices[0]), Vertices, sizeof(Vertices)/sizeof(Vertices[0]));
```
- 定义三个全局变量用于获得Uniform变量的index索引

```
GLuint  m_WorldMatrixLocation;
GLuint m_dirLightDirectionLocation, m_dirLightDiffuseIntensityLocation;
```
- 获取Uniform变量的索引：

```
m_WorldMatrixLocation = glGetUniformLocation(ShaderProgram,"gWorld");
m_dirLightDirectionLocation = glGetUniformLocation(ShaderProgram, "gDirectionalLight.Direction");
assert(m_dirLightDirectionLocation != 0xFFFFFFFF);
m_dirLightDiffuseIntensityLocation = glGetUniformLocation(ShaderProgram, "gDirectionalLight.DiffuseIntensity");
assert(m_dirLightDiffuseIntensityLocation != 0xFFFFFFFF);
```
- 在主渲染函数Render()中传递Uniform变量的值到着色器，即传递漫反射的方向和强度，以及世界坐标的变化矩阵
[(在混合变换那一节讲到)
](https://blog.csdn.net/zach_z/article/details/80086523)
```
glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_FALSE, (const GLfloat*)p.GetWorldTrans());
Vector3f Direction;
CopyVector3(Direction, m_directionalLight.Direction);
NormalizeVector3(Direction);
glUniform3f(m_dirLightDirectionLocation, Direction[0], Direction[1], Direction[2]);
glUniform1f(m_dirLightDiffuseIntensityLocation, m_directionalLight.DiffuseIntensity);
```
- 把顶点法向量传送到着色器，这里启动一个顶点属性2用于顶点法向量，偏移量为一个三位向量(顶点位置)和一个二维向量纹理坐标：

```
glEnableVertexAttribArray(2);
...
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(Vector3f)+sizeof(Vector2f)));
...
glDisableVertexAttribArray(2);
```
在顶点着色器中使用layout(location=2)与之对应获取顶点法向量
- 顶点着色器增加了世界坐标变换矩阵Uniform变量的值和顶点法向量的值作为输入变量
shader.vs:

```
#version 330

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;

uniform mat4 gWVP;
uniform mat4 gWorld;

out vec2 TexCoord0;
out vec3 Normal0;

void main()
{
	gl_Position = gWVP * vec4(Position,1.0);
	TexCoord0=TexCoord;
	Normal0 = (gWorld * vec4(Normal,0.0)).xyz;
}
```
这里使用世界坐标变换矩阵和顶点法向量相乘获得具体位置的法向量，并且传递到片元着色器当中
- 片元着色器中扩充了了平行光结构体，并且使用Uniform变量接收从主程序中传来的漫射光的参数，并计算漫射光在物体上具体的颜色，结合环境光颜色与纹理颜色相乘得到最终颜色。shader.fs:

```
#version 330

in vec2 TexCoord0;
in vec3 Normal0;

out vec4 FragColor;

struct DirectionalLight
{
    vec3 Color;
    float AmbientIntensity;
	float DiffuseIntensity;
    vec3 Direction;
};

uniform DirectionalLight gDirectionalLight;
uniform sampler2D gSampler;

void main()
{
	vec4 AmbientColor = vec4(gDirectionalLight.Color,1.0f)*gDirectionalLight.AmbientIntensity;

	float DiffuseFactor = dot(normalize(Normal0),-gDirectionalLight.Direction);

	vec4 DiffuseColor;
	if(DiffuseFactor > 0){
		DiffuseColor = vec4(gDirectionalLight.Color,1.0f) * gDirectionalLight.DiffuseIntensity * DiffuseFactor;
	}
	else{
		DiffuseColor=vec4(0,0,0,0);
	}
	FragColor = texture2D(gSampler,TexCoord0)*(AmbientColor+DiffuseColor);
}
```
可以看到环境光的计算没有发生变化

```
vec4 AmbientColor = vec4(gDirectionalLight.Color,1.0f)*gDirectionalLight.AmbientIntensity;
```
随后计算法向量与光线入射方向的角度的余弦值，用点积来得到余弦值，两个单位向量长度的点积结果为夹角的余弦值，相关博文：
[OpenGL学习之路11----相机空间](https://blog.csdn.net/zach_z/article/details/80102219)

只用单位化法向量，因为传来的漫射光方向向量已经是单位化过了的

并且还需要将光源方向取反，这点其实已经在之前说过了，比如垂直入射到物体表面，入射角α应该为0°，然而法向量竖直向上是180°所以要取反一下

根据α的余弦值，即DiffuseFactor的值判断角度大小，如果是正则计算漫反射在物体上的光照颜色，如果余弦值为负，则证明入射方向垂直法向量或者在反面，则不产生作用

```
if(DiffuseFactor > 0){
		DiffuseColor = vec4(gDirectionalLight.Color,1.0f) * gDirectionalLight.DiffuseIntensity * DiffuseFactor;
	}
	else{
		DiffuseColor=vec4(0,0,0,0);
	}
```
最后根据纹理颜色，环境光颜色和漫反射颜色得到最终颜色：

```
FragColor = texture2D(gSampler,TexCoord0)*(AmbientColor+DiffuseColor);
```

## 运行结果
我们在主程序中假如按键控制来体验环境光和漫射光强度的变化

```
static void Keyboard(int key, int i, int y)
{
	pGameCamera->OnKeyboard(key);
	switch (key) {
	case GLUT_KEY_F1:
		m_directionalLight.AmbientIntensity += 0.05f;
		break;

	case GLUT_KEY_F2:
		m_directionalLight.AmbientIntensity -= 0.05f;
		break;

	case GLUT_KEY_F3:
		m_directionalLight.DiffuseIntensity += 0.05f;
		break;

	case GLUT_KEY_F4:
		m_directionalLight.DiffuseIntensity -= 0.05f;
		break;
	}
```
- F1键：增大环境光光强
- F2键：减小环境光光强
- F3键：增大漫反射光强
- F4键：减小漫反射光强

通过按键控制可以体验效果
![image](E:/笔记/Opengl/Diffuse_Lighting/4.png)