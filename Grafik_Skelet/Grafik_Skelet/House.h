#pragma once

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/glew.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

class House
{
public:
	House(void);
	virtual ~House(void);

	void init();
	void drawHouse(GLfloat front[], GLfloat back[], GLfloat sides[]);
private:
};

