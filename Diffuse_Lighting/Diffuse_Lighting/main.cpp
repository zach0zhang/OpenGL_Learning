#include <stdio.h>
#include <string>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <assert.h>

#include "opengl_math.h"
#include "opengl_pipeline.h"
#include "opengl_texture.h"
#include "opengl_camera.h"

#define Window_Width 1024
#define Window_Height 768

using namespace std;

GLuint VBO, IBO;
GLuint gWVPLocation, gSampler, m_WorldMatrixLocation;
GLuint m_dirLightColorLocation, m_dirLightAmbientIntensityLocation;
GLuint m_dirLightDirectionLocation, m_dirLightDiffuseIntensityLocation;

Camera *pGameCamera = NULL;
Texture *pTexture = NULL;
PersProjInfo gPersProjInfo;


const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

unsigned int Indices[] =
{
	0,3,1,
	1,3,2,
	2,3,0,
	0,1,2
};

struct DirectionalLight
{
	Vector3f Color;
	float AmbientIntensity;
	Vector3f Direction;
	float DiffuseIntensity;
};
struct DirectionalLight m_directionalLight;
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
//计算法向量
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
		//printf("%f %f %f\n", pVertices[i].m_normal[0], pVertices[i].m_normal[1], pVertices[i].m_normal[2]);
	}
	//system("pause");
}

static void CreateVertexBuffer()
{
	Vector3f Vertices3f[4];
	Vector2f Vertices2f[4];
	LoadVector3(Vertices3f[0], -1.0f, -1.0f, 0.5773f);	LoadVector2(Vertices2f[0], 0.0f, 0.0f);
	LoadVector3(Vertices3f[1], 0.0f, -1.0f, -1.15475f); LoadVector2(Vertices2f[1], 0.5f, 0.0f);
	LoadVector3(Vertices3f[2], 1.0f, -1.0f, 0.5773f);	LoadVector2(Vertices2f[2], 1.0f, 0.0f);
	LoadVector3(Vertices3f[3], 0.0f, 1.0f, 0.0f);		LoadVector2(Vertices2f[3], 0.5f, 1.0f);
	Vertex Vertices[4] = { Vertex(Vertices3f[0],Vertices2f[0]),
		Vertex(Vertices3f[1],Vertices2f[1]),
		Vertex(Vertices3f[2],Vertices2f[2]),
		Vertex(Vertices3f[3],Vertices2f[3]) };

	CalcNormals(Indices, sizeof(Indices)/sizeof(Indices[0]), Vertices, sizeof(Vertices)/sizeof(Vertices[0]));

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}
static void CreateIndexBuffer()
{

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}
bool ReadFile(const char* FileName, string &outFile)
{
	ifstream f(FileName);
	bool ret = FALSE;

	if (f.is_open()) {
		string line;
		while (getline(f, line)) {
			outFile.append(line);
			outFile.append("\n");
		}
		f.close();
		ret = TRUE;
	}
	else {
		fprintf(stderr, "%s,%d: unable to read file: %s\n", __FILE__, __LINE__, FileName);
		system("pasue");
	}
	return ret;
}
static void AddShader(GLuint ShaderProgram, const char* ShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj) {
		fprintf(stderr, "Error creating shader obj");
		system("pause");
		exit(1);
	}

	const GLchar* p[1];
	p[0] = ShaderText;
	GLint Length[1];
	Length[0] = strlen(ShaderText);
	glShaderSource(ShaderObj,1,p,Length);
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
	GLuint ShaderProgram = glCreateProgram();
	if (!ShaderProgram) {
		fprintf(stderr, "Error creating shader program");
		system("pause");
		exit(1);
	}

	string vs, fs;
	if (!ReadFile(pVSFileName,vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName,fs)) {
		exit(1);
	}

	AddShader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	AddShader(ShaderProgram,fs.c_str(),GL_FRAGMENT_SHADER);

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

	gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
	assert(gWVPLocation != 0xFFFFFFFF);
	gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
	assert(gSampler != 0xFFFFFFFF);
	m_dirLightColorLocation = glGetUniformLocation(ShaderProgram,"gDirectionalLight.Color");
	m_dirLightAmbientIntensityLocation = glGetUniformLocation(ShaderProgram,"gDirectionalLight.AmbientIntensity");
	m_WorldMatrixLocation = glGetUniformLocation(ShaderProgram,"gWorld");
	m_dirLightDirectionLocation = glGetUniformLocation(ShaderProgram, "gDirectionalLight.Direction");
	assert(m_dirLightDirectionLocation != 0xFFFFFFFF);
	m_dirLightDiffuseIntensityLocation = glGetUniformLocation(ShaderProgram, "gDirectionalLight.DiffuseIntensity");
	assert(m_dirLightDiffuseIntensityLocation != 0xFFFFFFFF);
}
static void Render()
{
	pGameCamera->OnRender();
	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;
	Scale += 0.1f;

	Pipeline p;
	p.Rotate(0.0f, Scale, 0.0f);
	p.WorldPos(0.0f, 0.0f, 3.0f);
	p.SetPerspectiveProj(gPersProjInfo);
	p.SetCamera(*pGameCamera);

	glUniformMatrix4fv(gWVPLocation, 1, GL_FALSE, (const GLfloat*)p.GetWVPTrans());
	glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_FALSE, (const GLfloat*)p.GetWorldTrans());

	//m_directionalLight.AmbientIntensity *= 1.0001f;
	glUniform3f(m_dirLightColorLocation, m_directionalLight.Color[0], m_directionalLight.Color[1], m_directionalLight.Color[2]);
	glUniform1f(m_dirLightAmbientIntensityLocation, m_directionalLight.AmbientIntensity);

	Vector3f Direction;
	CopyVector3(Direction, m_directionalLight.Direction);
	NormalizeVector3(Direction);
	glUniform3f(m_dirLightDirectionLocation, Direction[0], Direction[1], Direction[2]);
	glUniform1f(m_dirLightDiffuseIntensityLocation, m_directionalLight.DiffuseIntensity);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(Vector3f));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(Vector3f)+sizeof(Vector2f)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	pTexture->Bind(GL_TEXTURE);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glutSwapBuffers();
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
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(Window_Width, Window_Height);
	glutCreateWindow("Ambient Lighting");
	glutGameModeString("1028x1024@32");
	
	InitializeCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error init glew");
		system("pause");
		exit(1);
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW); 
	glCullFace(GL_BACK);    
	glEnable(GL_CULL_FACE);

	CreateVertexBuffer();
	CreateIndexBuffer();

	CompileShader();

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
	LoadVector3(CameraPos, 0.0f, 0.0f, -3.0f);
	LoadVector3(CameraTarget, 0.0f, 0.0f, 1.0f);
	LoadVector3(CameraUp, 0.0f, 1.0f, 0.0f);
	pGameCamera = new Camera(Window_Width, Window_Height, CameraPos, CameraTarget, CameraUp);

	LoadVector3(m_directionalLight.Color,1.0f, 1.0f, 1.0f);
	m_directionalLight.AmbientIntensity = 0.1f;
	m_directionalLight.DiffuseIntensity = 0.75f;
	LoadVector3(m_directionalLight.Direction,1.0f, 0.0, 0.0);

	glutMainLoop();

	return 0;












}