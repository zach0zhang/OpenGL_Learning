#include <stdio.h>
#include <string>
#include <gl/glew.h>]
#include <gl/freeglut.h>
#include <assert.h>
#include <fstream>

#include "opengl_math.h"
#include "opengl_pipeline.h"
#include "opengl_texture.h"

#define Window_Width 1024
#define Window_Height 768
using namespace std;

GLuint VBO,IBO;
GLuint gWVPLocation;
GLuint gSampler;
Camera* GameCamera = NULL;
Texture* pTexture = NULL;
PersProjInfo gPersProjInfo;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

struct Vertex
{
	Vector3f m_pos;
	Vector2f m_tex;

	Vertex() {}

	Vertex(Vector3f pos, Vector2f tex)
	{
		CopyVector3(m_pos, pos);
		CopyVector2(m_tex, tex);
	}
};

static void Render()
{
	GameCamera->OnRender();
	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;
	Scale += 0.1f;

	Pipeline p;
	p.Rotate(0.0f, Scale, 0.0f);
	p.WorldPos(0.0f, 0.0f, 3.0f);
	p.SetCamera(*GameCamera);
	p.SetPerspectiveProj(gPersProjInfo);

	glUniformMatrix4fv(gWVPLocation, 1, GL_FALSE, (const GLfloat*)p.GetWVPTrans());

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(Vector3f));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	pTexture->Bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glutSwapBuffers();
}
static void Keyboard(int key, int x, int y)
{
	GameCamera->OnKeyboard(key);
}
static void Mouse(int x, int y)
{
	GameCamera->OnMouse(x, y);
}
static void InitializeCallbacks()
{
	glutDisplayFunc(Render);
	glutIdleFunc(Render);

	glutPassiveMotionFunc(Mouse);
	glutSpecialFunc(Keyboard);
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

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}
static void CreateIndexBuffer()
{
	unsigned int Indices[] =
	{
		0,3,1,
		1,3,2,
		2,3,0,
		0,1,2
	};

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
		fprintf(stderr, "%s : %d unable to read file: %s", __FILE__, __LINE__, FileName);
		system("pause");
		exit(1);
	}
	return ret;
}
static void Addshader(GLuint ShaderProgram,const char* ShaderText,GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (!ShaderObj) {
		fprintf(stderr, "Error creating shder object");
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
		glGetShaderInfoLog(ShaderObj,sizeof(InfoLog),NULL,InfoLog);
		fprintf(stderr, "Error compiling shader object: '%s'", InfoLog);
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

	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	}

	Addshader(ShaderProgram,vs.c_str(),GL_VERTEX_SHADER);
	Addshader(ShaderProgram,fs.c_str(),GL_FRAGMENT_SHADER);

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
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(Window_Width, Window_Height);
	glutCreateWindow("Texture");
	glutGameModeString("1280x1024@32");
	InitializeCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "unable to init glew");
		system("pause");
		exit(1);
	}

	printf("GL version : '%s'", glGetString(GL_VERSION));

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
	gPersProjInfo.zNear = 1.0f;
	gPersProjInfo.zFar = 100.0f;

	Vector3f CameraPos, CameraTarget, CameraUp;
	LoadVector3(CameraPos, 0.0f, 0.0f, -4.0f);
	LoadVector3(CameraTarget, 0.0f, 0.0f, 1.0f);
	LoadVector3(CameraUp, 0.0f, 1.0f, 0.0f);
	GameCamera = new Camera(Window_Width, Window_Height, CameraPos, CameraTarget, CameraUp);

	glutMainLoop();

	return 0;
}