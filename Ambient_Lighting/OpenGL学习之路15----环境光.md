根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

## 环境光
光照在3D图形领域中有许多模型，常见的模型有：环境光、慢射光、镜面反射光、点光源等等，这次只对环境光进行学习和实现

环境光是一个没有光源、没有方向并且对场景中的所有物体产生相同的点亮效果的一种光

环境光可以理解成为世界的基本光，即我们这个三维空间的基本亮度。

**环境光有两个属性：**
- 颜色：即灯光的颜色，比如普通房子里白炽灯房子里就是白色的，特殊性场所红房子里粉红色调就是粉红的
- 强度：即光强，强度越大亮度也就越大

**环境光的计算：**
![image](E:/笔记/Opengl/Ambient_Lighting/1.png)

**我们用一个结构体来包括表示环境光的两个属性：**
```
struct DirectionalLight
{
	Vector3f Color;
	float AmbientIntensity;
};
```
- Color：是一个三维向量，三个分量分别代表r(红)、g(绿)、b(蓝)的比例,比如(1.0,0,0)就表示红色
- AmbientIntensity：即光强，最大为1.0
- 可以看到我们结构体的变量名是“平行光 DirectionalLight”，其实我们为的是实现一个平行光
- 结构体在这次只实现环境光，所以结构体中只有环境光的两个属性

平行光是有特定方向但是没有特定的光源，就好比我们现实世界的太阳光。

太阳光照射到物体上，物体受到光线直直照射的地方光线强，而其它角度光线较弱，背面几乎照不到，这些概念属于漫反射范畴。

我们的环境光属于，太阳照到我们物体上，而照到其他物体上也会反射光照到我们物体的背面，而这个基本的光就是我们的环境光。

## 环境光的实现
首先，要在主程序里将环境光的参数数值传递到着色器内，我们使用Uniform变量来传递参数的值

- 定义两个全局变量用于获得Uniform变量的index索引 main.cpp:
```
GLuint m_dirLightColorLocation, m_dirLightAmbientIntensityLocation;
```
- 获取Uniform变量的索引：

```
m_dirLightColorLocation = glGetUniformLocation(ShaderProgram,"gDirectionalLight.Color");
m_dirLightAmbientIntensityLocation = glGetUniformLocation(ShaderProgram,"gDirectionalLight.AmbientIntensity");
```
- 声明一个环境光结构体变量，并赋值
```
struct DirectionalLight
{
	Vector3f Color;
	float AmbientIntensity;
};
struct DirectionalLight m_directionalLight;


LoadVector3(m_directionalLight.Color,1.0f, 1.0f, 1.0f);
m_directionalLight.AmbientIntensity = 0.1f;
```
- 在主渲染函数Render()中，逐渐增大光强的值并把环境光的参数传递到着色器中：

```
m_directionalLight.AmbientIntensity *= 1.0001f;
glUniform3f(m_dirLightColorLocation, m_directionalLight.Color[0], m_directionalLight.Color[1], m_directionalLight.Color[2]);
glUniform1f(m_dirLightAmbientIntensityLocation, m_directionalLight.AmbientIntensity);
```
- 片元着色器中获取Uniform变量的值，并通过之前提到过的环境光计算的公式计算最终的颜色，shader.fs:

```
#version 330

in vec2 TexCoord0;

out vec4 FragColor;

struct DirectionalLight
{
    vec3 Color;
    float AmbientIntensity;
};

uniform DirectionalLight gDirectionalLight;
uniform sampler2D gSampler;

void main()
{
	FragColor = texture2D(gSampler,TexCoord0)*vec4(gDirectionalLight.Color,1.0f)*gDirectionalLight.AmbientIntensity;
}
```
## 运行结果
![image](E:/笔记/Opengl/Ambient_Lighting/2.png)
可以看到四面体由暗到亮
