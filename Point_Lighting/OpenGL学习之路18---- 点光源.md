根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步开始，记录学习历程

之前学习了三个基于平行光的光照模型，相关博文如下：

[OpenGL学习之路15----环境光](https://blog.csdn.net/zach_z/article/details/80188764)

[OpenGL学习之路16---- 漫射光](https://blog.csdn.net/Zach_z/article/details/80199389)

[OpenGL学习之路17---- 镜面反射光](https://blog.csdn.net/zach_z/article/details/80273392)

平行光没有起点，通过方向向量来表示，不会随着距离增大而衰减

## 点光源
点光源即为有起点的光源，有衰减现象(即离光源越远光线越弱)。生活中常见的点光源为电灯泡。

**点光源的衰减：**
光线强度离光源的距离平方成反比，数学原理公式如下：
![image](E:/笔记/Opengl/Point_Lighting/1.png)
我们可以根据这个公式来计算点光源的亮度，但是在3D图形中，这样的效果并不是很好。如果距离很近时，光强便接近无限大，所以将公式进行微整：
![image](E:/笔记/Opengl/Point_Lighting/2.png)
- 分母上添加了三个衰减参数：一个常量参数、一个线性参数和一个指数参数
- 当常量参数设为1，此时就算距离很小，也不会产生无限大的光强，起到滤波的作用，过滤掉不正常数据
- 线性参数用来控制缓慢的衰减效果
- 指数因子可以控制迅速的衰减 效果

**点光源的结构体表示：**

```
struct BaseLight
{
	Vector3f Color;
	float AmbientIntensity;
	float DiffuseIntensity;
};

struct PointLight : public BaseLight
{
	Vector3f Position;

	struct
	{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;

};
```
- 首先是跟平行光共同拥有的元素：光的颜色，环境光强度和漫射光强度
- 其次是点光源自身的位置
- 还有上面讲到的为了计算衰减后的光强的三个参数

## 实现点光源

因为之前所有光源实现部分都是在我们的主程序中实现，使得主程序有些臃肿，现在把光源实现部分都放在opengl_light.h中。

首先定义光源的结构体，用来表示光源：

```
struct BaseLight
{
	Vector3f Color;
	float AmbientIntensity;
	float DiffuseIntensity;

	BaseLight()
	{
		LoadVector3(Color,0.0f, 0.0f, 0.0f);
		AmbientIntensity = 0.0f;
		DiffuseIntensity = 0.0f;
	}
};

struct DirectionalLight:public BaseLight
{
	
	Vector3f Direction;

	DirectionalLight()
	{
		LoadVector3(Direction,0.0f, 0.0f, 0.0f);
	}
};

struct PointLight : public BaseLight
{
	Vector3f Position;

	struct
	{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;

	PointLight()
	{
		LoadVector3(Position, 0.0f, 0.0f, 0.0f);
		Attenuation.Constant = 1.0f;
		Attenuation.Linear = 0.0f;
		Attenuation.Exp = 0.0f;
	}
};
```
- 点光源和平行光都有的部分用BaseLight结构体来封装
- 平行光的特殊参数是方向向量，而点光源是光源位置，点光源通过自身位置和物体位置，在着色器中计算光线方向

定义一个Light类，用来获得Uniform变量的索引，传递Uniform变量的值。

```
class Light
{
public:
	Light(GLuint ShaderProgram)
	{
		m_shaderProg = ShaderProgram;
	}
	bool GetUniformLocations();
	void SetDirectionalLight(const DirectionalLight& Light);
	void SetEyeWorldPos(const Vector3f& EyeWorldPos);
	void SetMatSpecularIntensity(float Intensity);
	void SetMatSpecularPower(float Power);
	void SetPointLights(unsigned int NumLights, const PointLight* pLights);
	
private:
	static const unsigned int MAX_POINT_LIGHTS = 2;

	GLuint m_shaderProg;

	GLuint m_eyeWorldPosLocation;
	GLuint m_matSpecularIntensityLocation;
	GLuint m_matSpecularPowerLocation;
	GLuint m_numPointLightsLocation;

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Direction;
	} m_dirLightLocation;

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Position;
		struct
		{
			GLuint Constant;
			GLuint Linear;
			GLuint Exp;
		} Atten;
	} m_pointLightsLocation[MAX_POINT_LIGHTS];
};
```
- 私有变量都是为了获得Uniform变量索引的GLuint变量
- 函数GetUniformLocations()用来获得Uniform变量的索引

```
bool Light::GetUniformLocations()
{
	m_eyeWorldPosLocation = glGetUniformLocation(m_shaderProg, "gEyeWorldPos");
	m_dirLightLocation.Color = glGetUniformLocation(m_shaderProg, "gDirectionalLight.Base.Color");
	m_dirLightLocation.AmbientIntensity = glGetUniformLocation(m_shaderProg, "gDirectionalLight.Base.AmbientIntensity");
	m_dirLightLocation.Direction = glGetUniformLocation(m_shaderProg, "gDirectionalLight.Direction");
	m_dirLightLocation.DiffuseIntensity = glGetUniformLocation(m_shaderProg, "gDirectionalLight.Base.DiffuseIntensity");
	m_matSpecularIntensityLocation = glGetUniformLocation(m_shaderProg, "gMatSpecularIntensity");
	m_matSpecularPowerLocation = glGetUniformLocation(m_shaderProg, "gSpecularPower");
	m_numPointLightsLocation = glGetUniformLocation(m_shaderProg, "gNumPointLights");

	for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLightsLocation); i++)
	{
		char Name[128];
		memset(Name, 0, sizeof(Name));
		snprintf(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
		m_pointLightsLocation[i].Color = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
		m_pointLightsLocation[i].AmbientIntensity = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gPointLights[%d].Position", i);
		m_pointLightsLocation[i].Position = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
		m_pointLightsLocation[i].DiffuseIntensity = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
		m_pointLightsLocation[i].Atten.Constant = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
		m_pointLightsLocation[i].Atten.Linear = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
		m_pointLightsLocation[i].Atten.Exp = glGetUniformLocation(m_shaderProg, Name);

		if (m_pointLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
			m_pointLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
			m_pointLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
			m_pointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
			m_pointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
			m_pointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
			m_pointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
			return false;
		}
	}
	if (m_dirLightLocation.AmbientIntensity == INVALID_UNIFORM_LOCATION ||
		m_eyeWorldPosLocation == INVALID_UNIFORM_LOCATION ||
		m_dirLightLocation.Color == INVALID_UNIFORM_LOCATION ||
		m_dirLightLocation.DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
		m_dirLightLocation.Direction == INVALID_UNIFORM_LOCATION ||
		m_matSpecularIntensityLocation == INVALID_UNIFORM_LOCATION ||
		m_matSpecularPowerLocation == INVALID_UNIFORM_LOCATION ||
		m_numPointLightsLocation == INVALID_UNIFORM_LOCATION) {
		return false;
	}
	return true;
}
```
- 函数SetPointLights()用来传递点光源参数值

```
void Light::SetPointLights(unsigned int NumLights, const PointLight* pLights)
{
	glUniform1i(m_numPointLightsLocation, NumLights);

	for (unsigned int i = 0; i < NumLights; i++) {
		glUniform3f(m_pointLightsLocation[i].Color, pLights[i].Color[0], pLights[i].Color[1], pLights[i].Color[2]);
		glUniform1f(m_pointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
		glUniform1f(m_pointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
		glUniform3f(m_pointLightsLocation[i].Position, pLights[i].Position[0], pLights[i].Position[1], pLights[i].Position[2]);
		glUniform1f(m_pointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
		glUniform1f(m_pointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
		glUniform1f(m_pointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
	}
}
```
可以看出来把，每个点光源的每个参数都作为一个Unirom变量，如果有3个点光源则有3个结构体，每个结构体有7个参数，则就有21个Unifrom变量


在主渲染函数Render()中实例化两个点光源，并把对应参数值传递到着色器当中去：

```
    static float Scale = 0.0f;
	Scale += 0.0015f;

	PointLight pl[2];
	pl[0].DiffuseIntensity = 1.0f;
	LoadVector3(pl[0].Color,1.0f, 0.5f, 0.0f);
	LoadVector3(pl[0].Position, FieldWidth*(cosf(Scale) + 1.0f) / 2.0f, 1.0f, FieldDepth * (sinf(Scale) + 1.0f) / 2.0f);
	pl[0].Attenuation.Linear = 0.1f;
	pl[1].DiffuseIntensity = 1.0f;
	LoadVector3(pl[1].Color,0.0f, 0.5f, 1.0f);
	LoadVector3(pl[1].Position, 5.0f, 1.0f, FieldDepth * (sinf(Scale) + 1.0f) / 2.0f);
	pl[1].Attenuation.Linear = 0.1f;
	pLight->SetPointLights(2, pl);
```
这里设定点光源漫反射强度为1，一个点光源颜色为黄色(1.0,0.5,0.0)，做圆周运动，线性参数值为0.1；另一个点光源颜色为蓝绿色(0.0,0.5,1.0)，漫反射强度为1，在场景x轴中心做延z轴反复移动运动。

顶点着色器相比于上一节没有变化，片元着色器接收到主程序中传来的Uniform变量的值，计算得到像素最终颜色，shader.fs:

```
#version 330

const int MAX_POINT_LIGHTS=2;
in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;

out vec4 FragColor;

struct BaseLight
{   
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};   

struct DirectionalLight
{
	BaseLight Base;
	vec3 Direction;
};
struct Attenuation
{
    float Constant;
    float Linear;
    float Exp;
};

struct PointLight 
{
   BaseLight Base;
   vec3 Position;
   Attenuation Atten;
};   
          
uniform int gNumPointLights;
uniform DirectionalLight gDirectionalLight;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform float gMatSpecularIntensity;
uniform float gSpecularPower;  
uniform vec3 gEyeWorldPos;  
uniform sampler2D gSampler;

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)
{
    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0f);
    float DiffuseFactor = dot(Normal, -LightDirection);
    
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);
    
    if (DiffuseFactor > 0) {
        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0f);
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        if (SpecularFactor > 0)
        {
            SpecularFactor = pow(SpecularFactor, gSpecularPower);
            SpecularColor = vec4(Light.Color * gMatSpecularIntensity * SpecularFactor, 1.0f);
        }
        
    }
    return (AmbientColor + DiffuseColor + SpecularColor);
}       

vec4 CalcDirectionalLight(vec3 Normal)
{
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal); 
} 

vec4 CalcPointLight(int Index, vec3 Normal)
{
    vec3 LightDirection = WorldPos0 - gPointLights[Index].Position;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);
    vec4 Color = CalcLightInternal(gPointLights[Index].Base, LightDirection, Normal);
    float Attenuation =  gPointLights[Index].Atten.Constant +
                         gPointLights[Index].Atten.Linear * Distance + 
                         gPointLights[Index].Atten.Exp * Distance * Distance;
    return Color / Attenuation;
}
void main()
{
    vec3 Normal = normalize(Normal0);
    vec4 TotalLight = CalcDirectionalLight(Normal); 
    for (int i = 0 ; i < gNumPointLights ; i++) {
        TotalLight += CalcPointLight(i, Normal);
    }
    FragColor = texture2D(gSampler, TexCoord0.xy) * TotalLight;                             
}
```
- 首先定义光源的结构体，基础光BaseLight(包括光颜色、环境光强和漫反射光强)；平行光DirectionLight(包括基础光参数和方向向量)；点光源PointLight(包括基础光、点光源位置和衰弱参数)

- 函数CalcLightInternal()为计算平行光和点光源共同拥有的部分，即环境光颜色，漫反射颜色和镜面反射颜色

```
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)
{
    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0f);
    float DiffuseFactor = dot(Normal, -LightDirection);
    
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);
    
    if (DiffuseFactor > 0) {
        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0f);
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        if (SpecularFactor > 0)
        {
            SpecularFactor = pow(SpecularFactor, gSpecularPower);
            SpecularColor = vec4(Light.Color * gMatSpecularIntensity * SpecularFactor, 1.0f);
        }
        
    }
    return (AmbientColor + DiffuseColor + SpecularColor);
}  
```
这里计算部分跟上一节的片元着色器代码一样，先是计算环境光颜色(AmbientColor)，再用点积计算出入射角(DiffuseFactor)，再计算漫射光颜色(DiffuseColor)，再用视线向量(VertexToEye)和反射光向量(LightReflect)点积得到视线和反射光夹角(SpecularFactor),最后根据镜面反射强度(gMatSpecularIntensity)和镜面发光参数(gSpecularPower)得到镜面反射颜色(SpecularColor)
- 计算平行光函数CalcDirectionalLight()接收参数顶点法向量，把平行光参数带入计算共有函数中：

```
vec4 CalcDirectionalLight(vec3 Normal)
{
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal); 
} 
```
- 计算点光源函数CalcPointLight()接收点光源索引和顶点法向量作为参数，按照之前计算衰减的公式来计算出最终颜色：

```
vec4 CalcPointLight(int Index, vec3 Normal)
{
    vec3 LightDirection = WorldPos0 - gPointLights[Index].Position;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);
    vec4 Color = CalcLightInternal(gPointLights[Index].Base, LightDirection, Normal);
    float Attenuation =  gPointLights[Index].Atten.Constant +
                         gPointLights[Index].Atten.Linear * Distance + 
                         gPointLights[Index].Atten.Exp * Distance * Distance;
    return Color / Attenuation;
}
```
先根据点光源位置(gPointLights[Index].Position)和像素坐标位置(WorldPos0)计算出光线方向LightDirection()

使用GLSL内置函数length()来求得此时像素距离点光源的距离(Distance),再把光线方向向量单位化

把点光源基础光线参数，单位化后的方向向量和像素顶点法向量带入CalcLightInternal()函数得到基本颜色

再计算衰减分母，常数参数(Constant)+线性参数(Linear)\*距离+指数参数\*距离的平方

最后用基本颜色/衰减分母得到点光源最终颜色
- main()函数中，先把之前顶点着色器传来的顶点法向量单位化再带入计算平行光颜色函数CalcDirectionalLight()，再把每一个点光源带入计算点光源函数CalcPointLight()，最后根据纹理颜色和平行光颜色和每个点光源颜色叠加得出像素最终颜色


## 运行结果

