#pragma once
#include <GL/glew.h>

class myTexture
{
public:
	int width, height, pixelsize;
	GLuint texName;

	myTexture()
	{
	}

	myTexture(int w, int h, int ps)
	{
		width = w;
		height = h;
		pixelsize = ps;
	}
};
