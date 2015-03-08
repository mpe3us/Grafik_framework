#include "House.h"

void House::init()
{
	// Front Wall
	GLfloat frontWall[] = {
		0.0f, 0.0f, 54.0f,
		16.0f, 0.0f, 54.0f,
		16.0f, 10.0f, 54.0f,
		8.0f, 16.0f, 54.0f,
		0.0f, 10.0f, 54.0f
	};
	// Back Wall
	GLfloat backWall[] = {
		0.0f, 0.0f, 30.0f,
		16.0f, 0.0f, 30.0f,
		16.0f, 10.0f, 30.0f,
		8.0f, 16.0f, 30.0f,
		0.0f, 10.0f, 30.0f
	};
	// Sides 10 vertices - 5 lines
	GLfloat sides[] = {
		0.0f, 0.0f, 54.0f,
		0.0f, 0.0f, 30.0f,
		16.0f, 0.0f, 54.0f,
		16.0f, 0.0f, 30.0f,
		16.0f, 10.0f, 54.0f,
		16.0f, 10.0f, 30.0f,
		8.0f, 16.0f, 54.0f,
		8.0f, 16.0f, 30.0f,
		0.0f, 10.0f, 54.0f,
		0.0f, 10.0f, 30.0f
	};

	drawHouse(frontWall, backWall, sides);
}

void House::drawHouse(GLfloat front[], GLfloat back[], GLfloat sides[])
{
	// Genereate Vertex Array Object and buffers
	GLuint vao[3], vbo[3];
	glGenVertexArrays(3, vao);
	glGenBuffers(3, vbo); 

	// Bind the front vertices
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), front, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_LINE_LOOP, 0, 5);
	glDisableVertexAttribArray(0);

	// Bind the back vertices
	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), back, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_LINE_LOOP, 0, 5);
	glDisableVertexAttribArray(0);

	// Bind the sides vertices
	glBindVertexArray(vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(GLfloat), sides, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_LINE_LOOP, 0, 10);
	glDisableVertexAttribArray(0);
}

House::House(void)
{
}


House::~House(void)
{
}
