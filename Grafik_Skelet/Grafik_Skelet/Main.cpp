/** @file
* @Author: Christian Muf
* @Date:   2014-04-09 22:00:24
* @Last Modified time: 2015-02-15 17:09:12
* @Version: 1.2
*/

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/glew.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <fstream>
using std::string;
using std::ifstream;

#include "ShaderProgram.h"
#include "DotMaker.h"
#include "Line_rasterizer.h"
#include "Edge_rasterizer.h"
#include <algorithm>

// Struct for at coordinate/pixel
struct point_xy {
	int x,y;
};

/**
* Use this function to define keyboard control of the window.
* Find the SDL2 keycodes here:
* https://wiki.libsdl.org/SDL_Keycode
*/
static void controlScene(int key)
{
	if(key == SDLK_1) {}
}

static bool under(point_xy p1, point_xy p2)
{
	return p1.y < p2.y;
}

static void drawLine(int x1, int y1, int x2, int y2)
{
	// Init new rasterizer with two given points
	Line_rasterizer *rasterizer = new Line_rasterizer();
	rasterizer->init(x1,y1, x2,y2);
	// While there is more points (fragments) to draw, keep drawing them
	while(rasterizer->more_fragments())
	{
		rasterizer->next_fragment();
	}
}

static void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	// init the left and right rasterizer
	Edge_rasterizer *rasterizerLeft = new Edge_rasterizer();
	Edge_rasterizer *rasterizerRight = new Edge_rasterizer();

	if (y1 == y2)
	{
		// The triangle is horizontal
		// Check to see if the horizontal edge is in the top of the triangle or in bottom of the triangle
		if (y1 > y3)
		{
			rasterizerLeft->init(x3,y3, x1,y1);
		    rasterizerRight->init(x3,y3, x2,y2);
		}
		else
		{
			rasterizerLeft->init(x1,y1, x3,y3);
			rasterizerRight->init(x2,y2, x3,y3);
		}
	}
	else if (y1 == y3)
	{
		// The triangle is horizontal
		// Check to see if the horizontal edge is in the top of the triangle or in bottom of the triangle
		if (y1 > y2)
		{
			rasterizerLeft->init(x2,y2, x3,y3);
			rasterizerRight->init(x2,y2, x1,y1);
		}
		else
		{
			rasterizerLeft->init(x1,y1, x2,y2);
			rasterizerRight->init(x3,y3, x2,y2);
		}
	}
	else if ( y2 == y3)
	{
		// The triangle is horizontal
		// Check to see if the horizontal edge is in the top of the triangle or in bottom of the triangle
		if (y2 > y1)
		{
			rasterizerLeft->init(x1,y1, x2,y2);
			rasterizerRight->init(x1,y1, x3,y3);
		}
		else
		{
			rasterizerLeft->init(x2,y2, x1,y1);
			rasterizerRight->init(x3,y3, x1,y1);
		}
	}
	else
	{
		// The triangle is not horizontal

		point_xy p1, p2, p3;
		p1.x = x1;
		p1.y = y1;
		p2.x = x2;
		p2.y = y2;
		p3.x = x3;
		p3.y = y3;

		const int SIZE = 3;
		point_xy ys[SIZE] = {p1,p2,p3};
		std::sort(ys, ys + SIZE, under);

		rasterizerLeft->init(ys[0].x,ys[0].y, ys[2].x,ys[2].y);
		rasterizerRight->init(ys[0].x,ys[0].y, ys[1].x,ys[1].y, ys[2].x,ys[2].y); 

	}

	// Run next_fragment until there are no more fragments left
	while(rasterizerLeft->more_fragments() && rasterizerRight->more_fragments())
	{
		// Get current coordinates
		int left_x = rasterizerLeft->x();
		int left_y = rasterizerLeft->y();

		int right_x = rasterizerRight->x();
		int right_y = rasterizerRight->y();

		int cur_left_x = left_x;
		int cur_right_x = right_x;

		// Checks that left is actually left and right is right
		if(left_x > right_x)
		{
			// If not, then switch them
			cur_left_x = right_x;
			cur_right_x = left_x;
		}

		cur_left_x;
		// Draw the pixels between the left and right edge
		while(cur_left_x <= cur_right_x)
		{
			DotMaker::instance()->drawDot(cur_left_x, left_y);
			cur_left_x++;
		}

		// Go to next fragment
		rasterizerLeft->next_fragment();
		rasterizerRight->next_fragment();

	}

}

static void drawScene(GLuint shaderID)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(-1.0f);
	glDepthFunc(GL_GREATER);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(shaderID);

	DotMaker::instance()->setColor(1.0f, 1.0f, 1.0f);
	DotMaker::instance()->setScene(800, 600, 15, true);
	DotMaker::instance()->setColor(1.0f, 0.0f, 0.0f);

	//drawTriangle(1,1 , 10,15, 17,7);
	//drawTriangle(1,1 , 17,7, 10,15);
	//drawTriangle(17,7 , 1,1, 10,15);
	//drawTriangle(1,17 , 10,1, 17,17);
	//drawTriangle(0,0, 10,0 ,5,10);
	//drawTriangle(1,10,10,10,5,1);
	drawTriangle(1,1,11,3,5,10);

	glFlush();
}

static int fileRead(std::string const& filename, std::string* result)
{
	std::ifstream ifs(filename.data());
	if(ifs.is_open()) {
		*result = std::string((std::istreambuf_iterator<char>(ifs)),
							  (std::istreambuf_iterator<char>()));
		ifs.close();
		return 0;
	}
	return -1;
}

/**
* In case of errors in when running the executable,
* try to use the code that has been commented out.
*/
int main(int argc, char *argv[])
{
	//glewExperimental = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return -1;
	
	GLint width = 800;
	GLint height = 600;
	
	SDL_Window* window = SDL_CreateWindow(
		"Grafik 2015 framework",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_OPENGL);
	
	if(window == NULL)
		return -2;
	
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	
	if(glContext == NULL) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -3;
	}
	
	if(glewInit() != GLEW_OK) {
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -4;
	}

	std::string vs = "";
	std::string fs = "";

	if(fileRead("Shader.vert", &vs) < 0 ||
	   fileRead("Shader.frag", &fs) < 0)
	{
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -5;
	}

	GLuint shaderID = ShaderProgram::compileShaderProgram(vs, fs);

	// Create a Vertex Array Object
	GLuint vertexArrayID = 0;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	GLint done = 0;
	while(!done)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT) {
				done = 1;
			}
			else if((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.state == 0)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_QuitEvent event;
					event.type = SDL_QUIT;
					SDL_PushEvent((SDL_Event*)&event);
				}
				else {
					controlScene(event.key.keysym.sym);
				}
			}
		}

		drawScene(shaderID);
		SDL_GL_SwapWindow(window);
	}

	ShaderProgram::deleteShaderPrograms();
	
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
