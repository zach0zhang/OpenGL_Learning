#include "opengl_math.h"
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <stdio.h>
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define INVALID_UNIFORM_LOCATION 0xffffffff

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