#include "Triangle.h"

void Triangle::init()
{
	glm::vec3 vertices[] = {
		glm::vec3(-33.978017f, -34.985076f, 50.214926f),
		glm::vec3(84.192943f, -13.784394f, -50.214926f),
		glm::vec3(-16.236910f, 83.754546f, -50.214926f)
	};

	drawTriangle(vertices);
}

void Triangle::drawTriangle(glm::vec3 vertices[])
{
	// Genereate Vertex Array Object and buffers
	GLuint vao[1], vbo[2];
	glGenVertexArrays(1, vao);
	glGenBuffers(2, vbo); 

	// Bind the vertices
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);


	// Compute the normal vectors
	glm::vec3 normal_vectors[] =  { 
		glm::cross(vertices[1]-vertices[0], vertices[2]-vertices[0]),
		glm::cross(vertices[2]-vertices[1], vertices[0]-vertices[1]),
		glm::cross(vertices[0]-vertices[2], vertices[1]-vertices[2]),
	};

	// Bind the normal vectors
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), normal_vectors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Now draw the vertices
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
}

Triangle::Triangle(void)
{
}


Triangle::~Triangle(void)
{
}
