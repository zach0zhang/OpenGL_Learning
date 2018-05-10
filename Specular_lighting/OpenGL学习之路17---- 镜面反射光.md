根据教程：[ogldev](http://ogldev.atspace.co.uk/index.html)一步步从零开始，记录学习历程

之前完成环境光和漫射光的学习。环境光的计算只由光强来决定，场景中所有位置是同一亮度；漫射光的计算由光强和光的方向一同决定，相关博文如下：

[OpenGL学习之路15----环境光](https://blog.csdn.net/zach_z/article/details/80188764)

[OpenGL学习之路16---- 漫射光](https://blog.csdn.net/Zach_z/article/details/80199389)

## 镜面反射光
镜面反射光是当光线以一定的角度照射到物体表面后，从法线的另一侧堆成的角度反射出去的光线

所以这个光线就与相机的位置有关，如果相机的位置正好在反射光线上则能感受到反射光线的照射，如果相机离反射光线有一定的角度则感受到反射光线的照射效果稍弱。

观察下图：

![image](E:/笔记/Opengl/Specular_Lighting/1.png)

- I：入射光
- N：表面法向量
- R：反射光，与I关于法线对称，方向相反
- V：入射点到相机视点的向量，代表观察视线
- α：反射光R和观察视线V的夹角

由图可知当α(即反射光和观察视线的夹角)为0时，反射光的强度最大；随着α的角度不断增大，反射光的强度逐渐减小，当α为90°(即反射光和观察者视线垂直)时，感受不到反射光。

引入α的余弦值，正好可以表示这种关系：
![image](E:/笔记/Opengl/Specular_Lighting/2.png)
- 当α为0°时，α的余弦值为1，此时反射光强最大
- 随着α值逐渐增大，余弦值逐渐减小
- α为90°时，余弦值为0，继续增大值为负数则判定光强也为0

**计算夹角α：**
夹角α可以用，R(反射光)和V(观察视线)的点积来得到，关于点积的相关知识之前相机空间的博文中有讲到：
[OpenGL学习之路11----相机空间](https://blog.csdn.net/zach_z/article/details/80102219)

**计算观察视线V：** 可以通过相机位置与入射点位置的差得到。(单位化)

**计算反射光R：**
![image](E:/笔记/Opengl/Specular_Lighting/3.png)
根据图中所示，我们可以引入一个向量W来计算反射光R：
```
R = I + W
```
而W的长度即为，两倍的向量I在单位法向量N的相反向量(-N)上的投影的长度，而这个投影长度可以使用点积得到，W的方向跟单位法向量相同，所以向量W：
```
W = 2 * N * (-N · I)
```
所以，反射光结合两个等式可以得到：
```
R = I + 2 * N * (-N · I)
  = I - 2 * N * ( N · I)
```
GLSL中提供了一个内部函数reflect()，用来做上述运算，即求一个向量经过法线堆成后的反射向量

**镜面反射的最终公式：**
![image](E:/笔记/Opengl/Specular_Lighting/4.png)
- 将光的颜色与物体表面颜色相乘
- M：代表材料的镜面反射强度，这是由材料的物理特性决定的(比如木头就为0，金属的话这个值就大)
- p：镜面发光参数，用来增强反射光区域边缘强度，这个值也是材料物体特性决定的

## 镜面反射光的实现
在之前平行光结构体的基础上，增加两个属性，分别代表我们的镜面反射强度M和镜面发光参数p：

```
struct DirectionalLight
{
	Vector3f Color;
	float AmbientIntensity;
	Vector3f Direction;
	float DiffuseIntensity;

	float SpecularIntensity; // 镜面反射强度
	float SpecularPower;     //镜面发光参数 
};

```
定义三个GLuint类型的全局变量，以便获得Uniform变量的Index索引：

```
GLuint m_eyeWorldPosLocation;		//相机视点
GLuint m_matSpecularIntensityLocation; // 镜面反射强度
GLuint m_matSpecularPowerLocation;		//镜面发光参数
```
获取Uniform变量的索引：

```
m_eyeWorldPosLocation = glGetUniformLocation(ShaderProgram,"gEyeWorldPos");
m_matSpecularIntensityLocation = glGetUniformLocation(ShaderProgram,"gDirectionalLight.SpecularIntensity");
m_matSpecularPowerLocation = glGetUniformLocation(ShaderProgram, "gDirectionalLight.SpecularPower");
```
在主函数中给镜面反射强度和镜面发光参数赋初值：

```
m_directionalLight.SpecularIntensity = 1.0f;
m_directionalLight.SpecularPower = 32.0f;
```
在主渲染函数Render()中传递三个Uniform变量的值，分别是相机视点的位置，镜面反射强度和镜面发光参数：

```
Vector3f EyeWorldPos;
CopyVector3(EyeWorldPos, pGameCamera->GetPos());
glUniform3f(m_eyeWorldPosLocation, EyeWorldPos[0], EyeWorldPos[1], EyeWorldPos[2]);
glUniform1f(m_matSpecularIntensityLocation, m_directionalLight.SpecularIntensity);
glUniform1f(m_matSpecularPowerLocation, m_directionalLight.SpecularPower);
```
在顶点着色器中计算每个像素的世界坐标位置并作为输出变量传递到片元着色器中，**shader.vs:**

```
#version 330

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;

uniform mat4 gWVP;
uniform mat4 gWorld;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;   

void main()
{
	gl_Position = gWVP*vec4(Position,1.0);
	TexCoord0 = TexCoord;
	Normal0= (gWorld * vec4(Normal,0.0)).xyz;
	WorldPos0 = (gWorld * vec4(Position, 1.0)).xyz;  
}
```
main()函数中的最后一句即为计算，每个像素点的世界坐标位置

最后在片元着色器中计算最终的颜色。shader.fs:

```
#version 330

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;

out vec4 FragColor;

struct DirectionalLight
{
	vec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;
	vec3 Direction;
	float SpecularIntensity; // 镜面反射强度
	float SpecularPower;     //镜面发光参数 
};

uniform DirectionalLight gDirectionalLight;
uniform sampler2D gSampler;
uniform vec3 gEyeWorldPos;                                                          
  

void main()
{
	vec4 AmbientColor = vec4(gDirectionalLight.Color * gDirectionalLight.AmbientIntensity, 1.0f);
    vec3 LightDirection = -gDirectionalLight.Direction;
    vec3 Normal = normalize(Normal0);   
    
    float DiffuseFactor = dot(Normal, LightDirection);   
    
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);            
    
    if (DiffuseFactor > 0) {           
        DiffuseColor = vec4(gDirectionalLight.Color * gDirectionalLight.DiffuseIntensity * DiffuseFactor, 1.0f);
        
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0); 
        vec3 LightReflect = normalize(reflect(gDirectionalLight.Direction, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);                      
        if (SpecularFactor > 0) {
            SpecularFactor = pow(SpecularFactor, gDirectionalLight.SpecularPower);
            SpecularColor = vec4(gDirectionalLight.Color * gDirectionalLight.SpecularIntensity * SpecularFactor, 1.0f);
        }                                                                           
    }                   
    FragColor = texture2D(gSampler, TexCoord0.xy) * (AmbientColor + DiffuseColor + SpecularColor);
}
```
- 环境光(AmbientColor)计算没有发生变化,先计算出环境光颜色
- 再对入射光线取反，这里原因在上一节有讲，因为比如垂直入射到物体表面，入射角α应该为0°，然而法向量竖直向上是180°所以要取反一下
- 对每个像素的顶点法线(Normal0)单位化，因为经过顶点着色器计算后不是一定是单位向量
- 漫射光计算也跟上一节一样，计算出入射角与法线的角度余弦值(DiffuseFactor)，通过入射光向量和顶点法向量的点积得到
- 如果入射角小于90°即(余弦值DiffuseFactor大于0)，则计算出漫射光颜色(DiffuseColor)，即光的颜色乘慢射光强乘入射角余弦值； 入射角如果大于90°则光线没有影响即光颜色为(0,0,0,0)
- **按照前面推理的镜面反射光计算方法来计算镜面反射光:**

得到观察视线的单位向量：相机位置-像素位置，再单位化：
```
vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0); 
```
得到入射光经过平面得到的反射光向量：使用GLSL内置函数得到入射光与法向量对应的反射光向量,再单位化：
```
vec3 LightReflect = normalize(reflect(gDirectionalLight.Direction, Normal));
```
得到观察视线与反射光之间的夹角：点积后得到：
```
float SpecularFactor = dot(VertexToEye, LightReflect); 
```
如果夹角小于90°，则正面镜面反射光有效，此时根据上面的镜面反射光计算公式来得出镜面反射光颜色：
```
if (SpecularFactor > 0) {
    SpecularFactor = pow(SpecularFactor, gDirectionalLight.SpecularPower);
    SpecularColor = vec4(gDirectionalLight.Color * gDirectionalLight.SpecularIntensity * SpecularFactor, 1.0f);
}  
```
- 最后根据纹理颜色，环境光颜色、漫射光颜色和镜面反射光颜色共同得出像素颜色：
```
FragColor = texture2D(gSampler, TexCoord0.xy) * (AmbientColor + DiffuseColor + SpecularColor);
```
## 运行结果
在之前调环境光和漫射光的基础上，增加了调镜面反射强度和镜面发光参数的按键，来观察变化

光从(1,0,0)入射时：
![image](E:/笔记/Opengl/Specular_Lighting/5.png)

光从(1,1,0)入射时：
![image](E:/笔记/Opengl/Specular_Lighting/6.png)

并且也可以通过调整镜面发光参数可以看出来，镜面发光参数越大，反射的面积集中，镜面发光参数越小，反射的光越发散
