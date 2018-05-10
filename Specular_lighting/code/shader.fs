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