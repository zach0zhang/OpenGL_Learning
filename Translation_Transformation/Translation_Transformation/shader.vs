#version 330

layout (location=0) in vec3 Position;

uniform mat4 gWorld1;
uniform mat4 gWorld2;
uniform mat4 gWorld3;


void main()
{
	gl_Position =  gWorld1*gWorld2*gWorld3 * vec4(Position,1.0);
}