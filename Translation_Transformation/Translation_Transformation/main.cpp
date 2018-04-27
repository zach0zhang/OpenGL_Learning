#include <stdio.h>
#include <string>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <assert.h>
#include "opengl_math.h"

using namespace std;

GLuint VBO;
GLint gWorldLocation1;
GLint gWorldLocation2;
GLint gWorldLocation3;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

bool ReadFile(const char* pFileName, string &outFile)
{
	ifstream f(pFileName);

	bool ret = false;

	if (f.is_open()) {
		string line;
		while (getline(f, line)) {
			outFile.append(line);
			outFile.append("\n");
		}
		f.close();
		ret = true;
	}
	else {
		fprintf(stderr, "%s:%d: unable to open file '%s'\n", __FILE__, __LINE__, pFileName);
		system("pause");
	}
	return ret;
}

static void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;

	Scale += 0.001f;

	Matrix44f World1;
	Matrix44f World2;
	Matrix44f World3;
	/*  Matrix44f   4*4        */
	/*  0  4  8   12           */
	/*  1  5  9   13           */
	/*  2  6  10  14           */
	/*  3  7  11  15           */
	/*
	World[0] = 1.0f; World[4] = 0.0f; World[8] = 0.0f;  World[12] = sinf(Scale);
	World[1] = 0.0f; World[5] = 1.0f; World[9] = 0.0f;  World[13] = 0.0f;
	World[2] = 0.0f; World[6] = 0.0f; World[10] = 1.0f; World[14] = 0.0f;
	World[3] = 0.0f; World[7] = 0.0f; World[11] = 0.0f; World[15] = 1.0f;
	*/
	TranslationMatrix44(World1, sinf(Scale), 0.0f, 0.0f);

	RotationMatrix44(World2, Scale, 0, 0, 1);

	Vector3f vScale;
	LoadVector3(vScale, sinf(Scale), sinf(Scale), sinf(Scale));
	ScaleMatrix44(World3,vScale[0], vScale[1], vScale[2]);

	glUniformMatrix4fv(gWorldLocation1, 1, GL_FALSE, &World1[0]);//GL_FLASE: 数组列优先
	glUniformMatrix4fv(gWorldLocation2, 1, GL_FALSE, &World2[0]);
	glUniformMatrix4fv(gWorldLocation3, 1, GL_FALSE, &World3[0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
	glutSwapBuffers();
}


static void CreateVertexBuffer()
{
	Vector3f Vertices[3];

	LoadVector3(Vertices[0], -1.0f, -1.0f, 0.0f);
	LoadVector3(Vertices[1], 1.0f, -1.0f, 0.0f);
	LoadVector3(Vertices[2], 0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		system("pause");
		exit(1);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		system("pause");
		exit(1);
	}
	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders() 
{
	GLuint ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName,vs)) {
		exit(1);
	}
	if (!ReadFile(pFSFileName,fs)) {
		exit(1);
	}

	AddShader(ShaderProgram, vs.c_str(),GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(),GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	glUseProgram(ShaderProgram);
	
	gWorldLocation1 = glGetUniformLocation(ShaderProgram, "gWorld1");
	assert(gWorldLocation1 != 0xFFFFFFFF);

	gWorldLocation2 = glGetUniformLocation(ShaderProgram, "gWorld2");
	assert(gWorldLocation2 != 0xFFFFFFFF);
	
	gWorldLocation3 = glGetUniformLocation(ShaderProgram, "gWorld3");
	assert(gWorldLocation3 != 0xFFFFFFFF);
}


static void InitializeGlutCallbacks()
{
	glutDisplayFunc(Render);

	glutIdleFunc(Render);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Transformation");

	InitializeGlutCallbacks();

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		system("pause");
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();

	CompileShaders();

	glutMainLoop();

	return 0;


}