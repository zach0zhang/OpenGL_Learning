#include <stdio.h>
#include <fstream>
#include <string>
#include <assert.h>
#include <gl/glew.h>
#include <gl/freeglut.h>

#include "opengl_camera.h"
#include "opengl_math.h"
#include "opengl_pipeline.h"
#include "opengl_texture.h"
#include "opengl_light.h"

#define Window_Width 1024
#define Window_Height 768

using namespace std;

const char* PVSFileName = "shader.vs";
const char* PFSFileName = "shader.fs";

GLuint ShaderProgram;
GLuint VBO, IBO;
GLuint gWVPLocation, gSampler, m_WorldMatrixLocation;



Camera *pGameCamera = NULL;
Texture *pTexture = NULL;
PersProjInfo gPersProjInfo;
Light *pLight = NULL;

DirectionalLight m_directionalLight;


struct Vertex
{
	Vector3f m_pos;
	Vector2f m_tex;
	Vector3f m_normal;

	Vertex() {}

	Vertex(Vector3f pos, Vector2f tex,Vector3f normal)
	{
		CopyVector3(m_pos, pos);
		CopyVector2(m_tex, tex);
		CopyVector3(m_normal, normal);
	}
};
static const float FieldDepth = 20.0f;
static const float FieldWidth = 10.0f;
static void Render()
{
	pGameCamera->OnRender();
	glClear(GL_COLOR_BUFFER_BIT);

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

	Pipeline p;
	p.Rotate(0.0f, 0.0f, 0.0f);
	p.WorldPos(0.0f, 0.0f, 1.0f);
	p.SetPerspectiveProj(gPersProjInfo);
	p.SetCamera(*pGameCamera);

	glUniformMatrix4fv(gWVPLocation, 1, GL_FALSE, (const GLfloat*)p.GetWVPTrans());
	glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_FALSE, (const GLfloat*)p.GetWorldTrans());

	pLight->SetDirectionalLight(m_directionalLight);

	Vector3f EyeWorldPos;
	CopyVector3(EyeWorldPos, pGameCamera->GetPos());
	pLight->SetEyeWorldPos(EyeWorldPos);
	pLight->SetMatSpecularIntensity(0.0f);
	pLight->SetMatSpecularPower(0);


	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(Vector3f));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(Vector3f) + sizeof(Vector2f)));
	pTexture->Bind(GL_TEXTURE);

	glDrawArrays(GL_TRIANGLES, 0,6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glutSwapBuffers();
}


static void CreateVertexBuffer()
{
	Vector3f  Normal;
	LoadVector3(Normal, 0.0f, 1.0f, 0.0f);
	Vector3f Vertices3f[6];
	Vector2f Vertices2f[6];
	LoadVector3(Vertices3f[0], 0.0f, 0.0f, 0.0f);				LoadVector2(Vertices2f[0], 0.0f, 0.0f);
	LoadVector3(Vertices3f[1], 0.0f, 0.0f, FieldDepth);			LoadVector2(Vertices2f[1], 0.5f, 1.0f);
	LoadVector3(Vertices3f[2], FieldWidth, 0.0f, 0.0f);			LoadVector2(Vertices2f[2], 1.0f, 0.0f);
	LoadVector3(Vertices3f[3], FieldWidth, 0.0f, 0.0f);			LoadVector2(Vertices2f[3], 1.0f, 0.0f);
	LoadVector3(Vertices3f[4], 0.0f, 0.0f, FieldDepth);			LoadVector2(Vertices2f[4], 0.0f, 1.0f);
	LoadVector3(Vertices3f[5], FieldWidth, 0.0f, FieldDepth);	LoadVector2(Vertices2f[5], 1.0f, 1.0f);
	Vertex Vertices[6] = { Vertex(Vertices3f[0],Vertices2f[0],Normal),
		Vertex(Vertices3f[1],Vertices2f[1],Normal),
		Vertex(Vertices3f[2],Vertices2f[2],Normal),
		Vertex(Vertices3f[3],Vertices2f[3],Normal),
		Vertex(Vertices3f[4],Vertices2f[4],Normal),
		Vertex(Vertices3f[5],Vertices2f[5],Normal) };

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static bool ReadFile(const char* FileName, string &outFile)
{
	bool ret = false;
	ifstream f(FileName);
	if (f.is_open()) {
		string line;
		while (getline(f, line)) {
			outFile.append(line);
			outFile.append("\n");
		}
		f.close();
		ret = true;
	}
	return ret;
}
static void AddShader(GLuint ShaderProgram, const char* ShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj)
	{
		fprintf(stderr, "Error creating shader object");
		system("pause");
		exit(1);
	}

	const GLchar* p[1];
	p[0] = ShaderText;
	GLint Length[1];
	Length[0] = strlen(ShaderText);
	glShaderSource(ShaderObj, 1, p, Length);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader obj:'%s'\n", InfoLog);
		system("pause");
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}
static void CompileShader()
{
	ShaderProgram = glCreateProgram();
	if (!ShaderProgram)
	{
		fprintf(stderr, "Error creating shader program");
		system("pause");
		exit(1);
	}

	string vs, fs;
	if (!ReadFile(PVSFileName, vs)) {
		exit(1);
	}
	if (!ReadFile(PFSFileName, fs)) {
		exit(1);
	}

	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLchar ErrorLog[1024] = { 0 };
	GLint Success;

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'", ErrorLog);
		system("pause");
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Valied to shader program:'%s'", ErrorLog);
		system("pause");
		exit(1);
	}
	glUseProgram(ShaderProgram);
	
}

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
}

static void Mouse(int x, int y)
{
	pGameCamera->OnMouse(x, y);
}

static void InitializeCallbacks()
{
	glutDisplayFunc(Render);
	glutIdleFunc(Render);

	glutSpecialFunc(Keyboard);
	glutPassiveMotionFunc(Mouse);
}

int main(int argc,char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(Window_Width, Window_Height);
	glutCreateWindow("Point Lighting");
	glutGameModeString("1028x1024@32");

	InitializeCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error init the glew");
		system("pasue");
		exit(1);
	}

	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	CreateVertexBuffer();

	CompileShader();

	pLight = new Light(ShaderProgram);

	gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
	assert(gWVPLocation != 0xFFFFFFFF);
	gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
	assert(gSampler != 0xFFFFFFFF);
	m_WorldMatrixLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	pLight->GetUniformLocations();

	glUniform1i(gSampler, 0);

	pTexture = new Texture(GL_TEXTURE_2D, "test.png");
	if (!pTexture->Load()) {
		return 1;
	}

	gPersProjInfo.FOV = 60.0f;
	gPersProjInfo.Height = Window_Height;
	gPersProjInfo.Width = Window_Width;
	gPersProjInfo.zFar = 100.0f;
	gPersProjInfo.zNear = 1.0f;

	Vector3f CameraPos, CameraTarget, CameraUp;
	LoadVector3(CameraPos, 5.0f, 1.0f, -3.0f);
	LoadVector3(CameraTarget, 0.0f, 0.0f, 1.0f);
	LoadVector3(CameraUp, 0.0f, 1.0f, 0.0f);
	pGameCamera = new Camera(Window_Width, Window_Height, CameraPos, CameraTarget, CameraUp);


	LoadVector3(m_directionalLight.Color, 1.0f, 1.0f, 1.0f);
	m_directionalLight.AmbientIntensity = 0.1f;
	m_directionalLight.DiffuseIntensity = 0.75f;
	LoadVector3(m_directionalLight.Direction, 1.0f, 1.0, 0.0);


	glutMainLoop();

	return 0;

}