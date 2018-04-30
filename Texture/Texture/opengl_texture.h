#ifndef __OPENGL_TEXTURE_H
#define __OPENGL_TEXTURE_H
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture
{
public:
	Texture(GLenum TextureTarget, const std::string& FileName)
	{
		m_textureTarget = TextureTarget;
		m_fileName = FileName;
	}
	bool Load()
	{
		int width, height, nrChannels;
		unsigned char *data = stbi_load(m_fileName.c_str(),&width, &height, &nrChannels, 0);
		glGenTextures(1, &m_textureObj);
		glBindTexture(m_textureTarget, m_textureObj);
		glTexImage2D(m_textureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(m_textureTarget, 0);

		return true;
	}
	void Bind(GLenum TextureUnit)
	{
		glActiveTexture(TextureUnit);
		glBindTexture(m_textureTarget, m_textureObj);
	}
private:
	std::string m_fileName;
	GLenum m_textureTarget;
	GLuint m_textureObj;
};
#endif