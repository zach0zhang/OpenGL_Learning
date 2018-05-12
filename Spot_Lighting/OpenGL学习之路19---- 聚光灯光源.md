根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步开始，记录学习历程

## 聚光灯光源
聚光灯光源是结合了平行光方向的特性和点光源随距离衰减的特性，并且离光源越远，照亮圆形区域越大，即光源呈锥形。 在现实中一个例子为手电筒。

如图所示，即为聚光灯光源的锥形效果：
![image](E:/笔记/Opengl/Spot_Lighting/1.png)
- L为光源方向
- 我们只想让光源照亮红色夹角之间的区域，即2α的角度
- V为光源到某个像素的向量
- 如果β大于α则该像素不位于照亮区域内
- 为了得到夹角的余弦值，可以使用向量间点积来得到

如果按照在区域内就能照亮，区域外就不能，则会在照亮区域和不照亮区域之间形成很明显的边界，没有过渡的效果。

聚光灯光源从照亮区域的中心向边缘慢慢衰减，如果L和V向量重合时，夹角为0，点积(夹角余弦值)为1，随着夹角增大，余弦值减小。但是夹角从0到一个比较小的角，余弦值变化很缓慢，衰减不明显。

比如聚光灯夹角为20°，余弦值为0.939，[0.939，1]这个变化范围太小，不好作为衰减参数，应该使得衰减参数范围为[0,1]，所以把余弦值范围映射到[0,1]方法如下：
![image](E:/笔记/Opengl/Spot_Lighting/2.png)

计算原理即为：使用大范围——[0,1],和小范围——[cosα，1]的比例——(1-0)/(1-cosα)=d

对小范围进行映射扩张，如果是α<=β<=1 —— 1-(1-cosβ)*((1-0)/(1-cosα))

## 实现聚光灯光源

首先定义聚光灯光源的结构体，聚光灯光源集成点光源的结构体，并且多出了光源的方向向量和照亮的夹角两个参数
opengl_light.h:
```
struct SpotLight : public PointLight
{
	Vector3f Direction;
	float Cutoff;

	SpotLight()
	{
		LoadVector3(Direction,0.0f, 0.0f, 0.0f);
		Cutoff = 0.0f;
	}
};
```
之后在Light类中加入用于获取聚光灯光源Uniform变量的索引
opengl_light.h:
```
GLuint m_numSpotLightsLocation;
struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Position;
		GLuint Direction;
		GLuint Cutoff;
		struct {
			GLuint Constant;
			GLuint Linear;
			GLuint Exp;
		} Atten;
	} m_spotLightsLocation[MAX_SPOT_LIGHTS];
```
在Light类中获取Uniform变量的方法中加入获取聚光灯光源的代码
opengl_light.cpp:
```
for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_spotLightsLocation); i++) {
		char Name[128];
		memset(Name, 0, sizeof(Name));
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
		m_spotLightsLocation[i].Color = glGetUniformLocation(m_shaderProg,Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
		m_spotLightsLocation[i].AmbientIntensity = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Position", i);
		m_spotLightsLocation[i].Position = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
		m_spotLightsLocation[i].Direction = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
		m_spotLightsLocation[i].Cutoff = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
		m_spotLightsLocation[i].DiffuseIntensity = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
		m_spotLightsLocation[i].Atten.Constant = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
		m_spotLightsLocation[i].Atten.Linear = glGetUniformLocation(m_shaderProg, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
		m_spotLightsLocation[i].Atten.Exp = glGetUniformLocation(m_shaderProg, Name);

		if (m_spotLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].Direction == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].Cutoff == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
			m_spotLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
			return false;
		}
	}
```
在主程序中的主渲染函数Render()中增加两个聚光灯光源
main.cpp:

```
    SpotLight sl[2];
	sl[0].DiffuseIntensity = 0.9f;
	LoadVector3(sl[0].Color,1.0f, 1.0f, 1.0f);
	CopyVector3(sl[0].Position, pGameCamera->GetPos());
	CopyVector3(sl[0].Direction, pGameCamera->GetTarget());
	sl[0].Attenuation.Linear = 0.1f;
	sl[0].Cutoff = 10.0f;

	sl[1].DiffuseIntensity = 0.9f;
	LoadVector3(sl[1].Color,1.0f, 1.0f, 1.0f);
	LoadVector3(sl[1].Position,5.0f, 3.0f, 10.0f);
	LoadVector3(sl[1].Direction,0.0f, -1.0f, 0.0f);
	sl[1].Attenuation.Linear = 0.1f;
	sl[1].Cutoff = 20.0f;
	pLight->SetSpotLights(2, sl);
```
- 可以看到增加了两个聚光灯光源
- 第一个位置在相机位置，随着相机移动而移动，从相机朝向射出，漫射光强度为0.9，线性衰弱为0.1,照射角度为10度
- 第二个位置固定在(5.0,3.0,10.0)，方向朝(0.0,-1.0f,0.0)，即在我们场景的中心上方朝正下方照射，照射角度为20度

顶点着色器没有变化，片元着色器接收主程序传来的聚光灯光源的Uniform变量来计算得到最终的像素颜色。

先是在片元着色器中定义了聚光灯光源的结构体，包括一个点光源的所有参数、一个方向向量和一个照射范围角度
shader.fs:
```
struct SpotLight
{
    PointLight Base;
    vec3 Direction;
    float Cutoff;
}; 
```
之后声明Uniform变量，接收从主程序传来的聚光灯光源个数和聚光灯光源参数
shader.fs:
```
uniform int gNumSpotLights;
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];  
```
增加计算聚光灯光源颜色的函数CalcSpotLight()
shader.fs:
```
vec4 CalcSpotLight(SpotLight l, vec3 Normal)
{
    vec3 LightToPixel = normalize(WorldPos0 - l.Base.Position);
    float SpotFactor = dot(LightToPixel, l.Direction);
    if (SpotFactor > l.Cutoff) {
        vec4 Color = CalcPointLight(l.Base, Normal);
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));
    }
    else {
        return vec4(0,0,0,0);
    }
} 
```
- 计算出聚光灯光源到像素点位置的方向向量LightToPixel(单位化)
- 将聚光灯光源照射方向和光源到像素的方向向量点积得到夹角的余弦值SpotFactor
- 如果夹角小于照射角范围，即余弦值大于照射角余弦值(SpotFactor > l.Cutoff)则计算聚光灯颜色，如果夹角大于照射范围，则聚光灯光源不影响，即颜色为(0,0,0,0)
- 先用计算点光源光照颜色函数CalcPointLight()计算出光源到像素点距离后的衰减颜色
- 再根据上面提到的公式，用光源到像素方向向量和光源照射方向的夹角计算出最终衰减颜色Color\*SpotFactor*1/(1-l.Cutoff)

最后在main()函数中根据纹理颜色、平行光颜色、点光源颜色和聚光灯光源颜色共同决定像素颜色
shader.fs:
```
void main()
{
    vec3 Normal = normalize(Normal0);
    vec4 TotalLight = CalcDirectionalLight(Normal);
    for (int i = 0 ; i < gNumPointLights ; i++) {
        TotalLight += CalcPointLight(gPointLights[i], Normal);
    }              
	for (int i = 0 ; i < gNumSpotLights ; i++) {
	    TotalLight += CalcSpotLight(gSpotLights[i], Normal);
	    
	}
	FragColor = texture2D(gSampler, TexCoord0.xy) * TotalLight;
    
}
```
## 运行结果
![image](E:/笔记/Opengl/Spot_Lighting/3.png)
可以看到一个正中心的聚光灯光源照射和一个随着相机移动而移动的聚光灯光源