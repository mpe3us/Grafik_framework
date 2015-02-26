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

#include <string>
#include <fstream>
using std::string;
using std::ifstream;

#include "ShaderProgram.h"
#include "DotMaker.h"
#include "Line_rasterizer.h"

/**
* Use this function to define keyboard control of the window.
* Find the SDL2 keycodes here:
* https://wiki.libsdl.org/SDL_Keycode
*/
static void controlScene(int key)
{
	if(key == SDLK_1) {}
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

	drawLine(0,0 , 17,10);

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
