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
#include "House.h"
#include "glmutils.h"
#include "Camera.h"
#include "Triangle.h"
#include "bezierpatch.h"
#include "readbezierpatches.h"

// Struct for a coordinate/pixel
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

// Checks which y-coordinate has the highest value (used for sorting the coordinates)
static bool greaterThan_y(point_xy p1, point_xy p2)
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

	// Creates new points (using the point_xy struct) from the given coordinates
	point_xy p1, p2, p3;
	p1.x = x1;	p1.y = y1;
	p2.x = x2;	p2.y = y2;
	p3.x = x3;	p3.y = y3;

	// Creates a new array containing the three points, and sorts them in ascending order by y-coordinate
	const int SIZE = 3;
	point_xy ys[SIZE] = {p1,p2,p3};
	std::sort(ys, ys + SIZE, greaterThan_y);

		// The following if-statements determine which kind of triangle we are dealing with
		if (ys[1].y == ys[0].y)
		{
			// The triangle is horizontal, and the triangle's "peak" is at the top
			rasterizerLeft->init(ys[0].x,ys[0].y, ys[2].x,ys[2].y);
		    rasterizerRight->init(ys[1].x,ys[1].y, ys[2].x,ys[2].y);
		}
		else if (ys[1].y == ys[2].y)
		{
			// The triangle is horizontal, and the triangle's "peak" is at the bottom
			rasterizerLeft->init(ys[0].x,ys[0].y, ys[1].x,ys[1].y);
		    rasterizerRight->init(ys[0].x,ys[0].y, ys[2].x,ys[2].y);
		}
		else
		{
			// The triangle is not horizontal
			rasterizerLeft->init(ys[0].x,ys[0].y, ys[2].x,ys[2].y);
			rasterizerRight->init(ys[0].x,ys[0].y, ys[1].x,ys[1].y, ys[2].x,ys[2].y); 
		}

	// Run next_fragment until there are no more fragments left
	while(rasterizerLeft->more_fragments() && rasterizerRight->more_fragments())
	{
		// Get current coordinates
		int left_x = rasterizerLeft->x(); int left_y = rasterizerLeft->y();
		int right_x = rasterizerRight->x();	int right_y = rasterizerRight->y();

		// Increment values used in upcoming loop
		int cur_left_x = left_x;
		int cur_right_x = right_x;

		// Checks that left is actually left and right is right
		if(left_x > right_x)
		{
			// If not, then switch them
			cur_left_x = right_x;
			cur_right_x = left_x;
		}

		// Draw the pixels between the left and right edge
		while(cur_left_x < cur_right_x)
		{
			DotMaker::instance()->drawDot(cur_left_x, left_y);
			cur_left_x++;
		}

		// Go to next fragment
		rasterizerLeft->next_fragment();
		rasterizerRight->next_fragment();
	}
}

// Surfaces and normals for the Klein Bottle
glm::vec3 klein_bot(float u, float v) {
  float x = (2.5f + 1.5f * cosf (v)) * cosf(u);
  float y = (2.5f + 1.5f * cosf(v)) * sinf(u);
  float z = -2.5f * sinf(v);
  return glm::vec3(x,y,z);
}
glm::vec3 klein_bot_n(float u, float v) {
  float x = (-6.25f - 3.75f * cosf(v)) * cosf(v) * cosf(u);
  float y = (-6.25f - 3.75f * cosf(v)) * cosf(v) * sinf(u);
  float z = (3.75f + 2.25f * cosf(v)) * sinf(v);
  return glm::vec3(x,y,z);
}
glm::vec3 klein_handle(float u, float v) {
  float x = 2.0f - 2.0f * cosf(v) + sinf(u);
  float y = cosf(u);
  float z = 3.0f * v;
  return glm::vec3(x,y,z);
}
glm::vec3 klein_handle_n(float u, float v) {
  float x = -3.0f * sinf(u);
  float y = -3.0f * cosf(u);
  float z = 2.0f * sinf(u) * sinf(v);
  return glm::vec3(x,y,z);
}
glm::vec3 klein_top(float u, float v) {
  float x = 2.0f + (2.0f + cosf(u)) * cosf (v);
  float y = sinf(u);
  float z = 3.0f * M_PI + (2.0f + cosf(u)) * sinf(v);
  return glm::vec3(x,y,z);
}
glm::vec3 klein_top_n(float u, float v) {
  float x = (2.0f + cosf(u)) * cosf(u) * cosf(v);
  float y = (2.0f + cosf(u)) * sinf(u);
  float z = (2.0f + cosf(u)) * cosf(u) * sinf(v);
  return glm::vec3(x,y,z);
}
glm::vec3 klein_middle(float u, float v) {
  float x = (2.5f + 1.5f * cosf(v)) * cosf(u);
  float y = (2.5f + 1.5f * cosf(v)) * sinf(u);
  float z = 3.0f * v;
  return glm::vec3(x,y,z);
}
glm::vec3 klein_middle_n(float u, float v) {
  float x = (7.5f + 4.5f * cosf(v)) * cosf(u);
  float y = (7.5f + 4.5f * cosf(v)) * sinf(u);
  float z = (3.75f + 2.25 * cosf(v)) * sinf(u);
  return glm::vec3(x,y,z);
}

// Data for Dini's Surface
float a = 1.5f;
float b = 0.5f;
// The d(u, v) function which represents a surface
glm::vec3 dini_d(float u, float v){
  float x = a * cosf(u) * sinf(v);
  float y = a * sinf(u) * sinf(v);
  float z = a * (cosf(v) + log(tanf(0.5f * v))) + b*u;
  return glm::vec3(x,y,z);
}
// The n(u, v) function which represents the normal of Dini's surface
glm::vec3 dini_n(float u, float v){
  float x1 = -a * sinf(u) * sinf(v);
  float y1 = a * cosf(u) * sinf(v);
  float z1 = b;

  float x2 = a * cosf(u) * cosf(v);
  float y2 = a * sinf(u) * cosf(v);
  float z2 = a * ( ((0.5f) / (sinf(0.5 * v) * cosf(0.5f * u) )) - sinf(v) );

  return glm::cross(glm::vec3(x1,y1,z1), glm::vec3(x2,y2,z2));
}

// Visualization of a general surface using the Sampling algorithm 
void generalSampling(
  glm::vec3 (*func[]  ) (float, float),  // Function(s) for position
  glm::vec3 (*Nfunc[] ) (float, float),  // Function(s) for normals
  float umin, float umax, // min/max values for u
  float vmin, float vmax, // min/max values for v
  int M, int N, // How many samples there is to be performed
  int funcCount ) // Total number of functions used to visualize the given object 
{
	// Array of all triangles
	std::vector <glm::vec3> vertices;
    // Array of all normals 
	std::vector <glm::vec3> normals;

	// Computes the initial values used for computing the points
	float deltaU, deltaV, u, v;
	deltaU = (umax - umin)/N;
	deltaV = (vmax - vmin)/M;
	u = umin;
	v = vmin;
	// Loop which finds every square in the surfaces and computes the two triangles in the square including the normals
	for (int i = 0; i < funcCount; i++) 
	{
		for (u = umin; u < umax; u += deltaU) 
		{
			for (v = vmin; v < vmax; v += deltaV) 
			{
				// The first triangle in the current surface
				vertices.push_back(func[i](u, v));
				vertices.push_back(func[i](u + deltaU, v));
				vertices.push_back(func[i](u, v + deltaV));
				// Compute the first set of normal vectors for shading purposes
				normals.push_back(Nfunc[i](u, v));
				normals.push_back(Nfunc[i](u + deltaU, v));
				normals.push_back(Nfunc[i](u, v + deltaV));
				// The second triangle in the current surface
				vertices.push_back(func[i](u + deltaU, v));
				vertices.push_back(func[i](u + deltaU, v + deltaV));
				vertices.push_back(func[i](u, v + deltaV));
				// Compute the second set of normal vectors for shading purposes
				normals.push_back(Nfunc[i](u + deltaU, v));
				normals.push_back(Nfunc[i](u + deltaU, v + deltaV));
				normals.push_back(Nfunc[i](u, v + deltaV));
			}
		}
	}

	// Now draw the object
	// Genereate Vertex Array Object and buffers
	GLuint vao[1], vbo[2];
	glGenVertexArrays(1, vao);
	glGenBuffers(2, vbo); 
	// Bind the vertices (triangles)
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Bind the vertices (normals)
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Now draw the all vertices
	glDrawArrays(GL_TRIANGLES, 0, normals.size());
	glDisableVertexAttribArray(0);

}

// Returns an array with all triangles and normals in the given Bezierpatch
static std::vector<std::vector<glm::vec3>> trianglesInPatch(std::vector<BezierPatch> patches)
{
    // Array of all triangles
	std::vector<glm::vec3> G_controlpoints;
	// Array of all normals 
	std::vector<glm::vec3> G_controlpointsNormals;
	
	// Loop which finds every square in the patches and computes the two triangles in the square including the normals
	for (int n=0; n<patches.size(); n++)
	{
		// The first triangle
		G_controlpoints.push_back(patches[n][1][1]);
		G_controlpoints.push_back(patches[n][1][4]);
		G_controlpoints.push_back(patches[n][4][4]);
		// Compute the first set of normal vectors for shading purposes
		G_controlpointsNormals.push_back( glm::normalize(glm::cross(patches[n][2][1]-patches[n][1][1], patches[n][1][2]-patches[n][1][1])) );
		G_controlpointsNormals.push_back( glm::normalize(glm::cross(patches[n][1][3]-patches[n][1][4], patches[n][2][4]-patches[n][1][4])) );
		G_controlpointsNormals.push_back( glm::normalize(glm::cross(patches[n][3][4]-patches[n][4][4], patches[n][4][3]-patches[n][4][4])) );
		// The second triangle
		G_controlpoints.push_back(patches[n][1][1]);
		G_controlpoints.push_back(patches[n][4][4]);
		G_controlpoints.push_back(patches[n][4][1]);
		// Compute the second set of normal vectors for shading purposes
		G_controlpointsNormals.push_back( glm::normalize(glm::cross(patches[n][2][1]-patches[n][1][1], patches[n][1][2]-patches[n][1][1])) );
		G_controlpointsNormals.push_back( glm::normalize(glm::cross(patches[n][3][4]-patches[n][4][4], patches[n][4][3]-patches[n][4][4])) );
		G_controlpointsNormals.push_back( glm::normalize(glm::cross(patches[n][4][2]-patches[n][4][1], patches[n][3][1]-patches[n][4][1])) );
	}

	// Array of arrays with all triangles and normals
	std::vector<std::vector<glm::vec3>> patchPointsAndNormals;
	patchPointsAndNormals.push_back(G_controlpoints);
	patchPointsAndNormals.push_back(G_controlpointsNormals);

	return patchPointsAndNormals;
}

// Visualization of bezier surfaces using the SubDivision algorithm 
static void bezierSubDivision(int subdivisions, const char *filename)
{
	std::vector<BezierPatch> bezierPatches;		
	// Read data file containing the Bezierpatch(es)
	ReadBezierPatches(filename, bezierPatches);
		
	// Compute the offset for the DLB patch/matrix
	glm::mat4x4 DLB(glm::vec4(8.0f, 0.0f, 0.0f, 0.0f),
					glm::vec4(4.0f, 4.0f, 0.0f, 0.0f),
					glm::vec4(2.0f, 4.0f, 2.0f, 0.0f),
					glm::vec4(1.0f, 3.0f, 3.0f, 1.0f));
	DLB /= 8.0f;
	// Compute the offset for the DRB patch/matrix
	glm::mat4x4 DRB(glm::vec4(1.0f, 3.0f, 3.0f, 1.0f),
					glm::vec4(0.0f, 2.0f, 4.0f, 2.0f),
					glm::vec4(0.0f, 0.0f, 4.0f, 4.0f),
					glm::vec4(0.0f, 0.0f, 0.0f, 8.0f));
	DRB /= 8.0f;

	std::vector<BezierPatch> subPatches;
	// Creates an array containing all sub-bezierpatches depending on the given number of subdivisions
	// 0 subdivisions means that the original patch is maintained and no subdivisions is performed at all
	while (subdivisions > 0)
	{
		for (int i=0; i<bezierPatches.size(); i++)
		{
			// Compute the 4 subpatches
			BezierPatch G11 = glm::transpose(DLB) * bezierPatches[i] * DLB;
			BezierPatch G12 = glm::transpose(DRB) * bezierPatches[i] * DLB;
			BezierPatch G21 = glm::transpose(DLB) * bezierPatches[i] * DRB;
			BezierPatch G22 = glm::transpose(DRB) * bezierPatches[i] * DRB; 
			// Push the 4 subpatches to the array
			subPatches.push_back(G11);
			subPatches.push_back(G12);
			subPatches.push_back(G21);
			subPatches.push_back(G22);
		}
		// Override the previous array which makes it possible to make further subdivisions if needed
		bezierPatches = subPatches;
		subPatches.clear();
		subdivisions--;
	}
	
	std::vector<std::vector<glm::vec3>> patchPointsAndNormals;
	std::vector<glm::vec3> Gtotal_controlpoints;
	std::vector<glm::vec3> Gtotal_normalvectors;
	// Collect triangles and normals from all patches and put them in the same array
	patchPointsAndNormals = trianglesInPatch(bezierPatches);
	// Splitting the normals triangles in two arrays
	Gtotal_controlpoints = patchPointsAndNormals[0];
	Gtotal_normalvectors = patchPointsAndNormals[1];
	
	// Now draw the object
	// Genereate Vertex Array Object and buffers
	GLuint vao[1], vbo[2];
	glGenVertexArrays(1, vao);
	glGenBuffers(2, vbo); 
	// Bind the vertices (triangles)
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, Gtotal_controlpoints.size() * 3 * sizeof(GLfloat), &Gtotal_controlpoints[0], GL_STATIC_DRAW);	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Bind the vertices (normals)
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, Gtotal_normalvectors.size() * 3 * sizeof(GLfloat), &Gtotal_normalvectors[0], GL_STATIC_DRAW);	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Now draw the all vertices
	glDrawArrays(GL_TRIANGLES, 0, Gtotal_normalvectors.size());
	glDisableVertexAttribArray(0);

}

static void drawScene(GLuint shaderID)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(-1.0f);
	glDepthFunc(GL_GREATER);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glUseProgram(shaderID);

	// Camera parameters
	/*glm::vec3 vrp(5.0f, 0.0f, 5.0f); // front view
	glm::vec3 vpn(cosf((30.0f*M_PI)/180.0f), 0.0f, sinf((30.0f*M_PI)/180.0f)); // front view */
	/*glm::vec3 vrp(5.0f, 0.0f, 6.0f); // Spin view
    glm::vec3 vpn(cosf((30.0f*M_PI)/180.0f), sinf(SDL_GetTicks()/1000.0f), sinf((30.0f*M_PI)/180.0f)); // Spin view */
	/*glm::vec3 vrp(5.0f, 0.0f, 5.0f); // side view
    glm::vec3 vpn(cosf((30.0f*M_PI)/180.0f), 10.0f, sinf((30.0f*M_PI)/180.0f)); // side view */
	glm::vec3 vrp(5.0f, 0.0f, 5.0f); 
	glm::vec3 vpn(0.9f, 0.9f, sinf((30.0f*M_PI)/180.0f)); 
	glm::vec3 vup(0.0f, 0.0f, 1.0f);
	glm::vec3 prp(0.0f, 0.0f, 50.0f);
	glm::vec2 lower_left( -6.0f, -6.0f);
	glm::vec2 upper_right(6.0f, 6.0f);
	float front_plane = 5.0f;
	float back_plane = -10.0f; 

	// Init the camera with the defined values
	Camera *camera = new Camera(vrp, vpn, vup, prp, lower_left, upper_right, front_plane, back_plane, 800, 600);

    // Set the shader matrixes 
    GLuint dir;
    dir = glGetUniformLocation(shaderID, "uModelMatrix");
    if (dir >= 0){
      glUniformMatrix4fv(dir, 1, GL_FALSE, &camera->ViewOrientation()[0][0]);
    }	
	glm::mat3 normalvectorMatrix = glm::inverseTranspose(glm::mat3(camera->ViewOrientation()));
    dir = glGetUniformLocation(shaderID, "normalvectorMatrix");
    if (dir >= 0){
      glUniformMatrix3fv(dir, 1, GL_FALSE, &normalvectorMatrix[0][0]);
    }
    glm::mat4 projectionMatrix = camera->ViewProjection();
    dir = glGetUniformLocation(shaderID, "projectionMatrix");
    if (dir >= 0){
      glUniformMatrix4fv(dir, 1, GL_FALSE, &projectionMatrix[0][0]);
    }
	
	// Material components
	glm::vec3 matAmbient = glm::vec3(0.0f, 0.75f, 1.0f) * 0.5f;
	glm::vec3 matDiffuse = glm::vec3(0.0f, 0.75f, 1.0f) * 0.75f;
	glm::vec3 matSpecular = glm::vec3(1.0f, 1.0f, 1.0f) * 0.9f;
	float matShiny = 20.0f;
	// light components
	//glm::vec4 lightPos = glm::vec4(266.395325f, 274.291267f, -43.696048f, 1.0f);
	glm::vec4 lightPos = glm::vec4(250.0f, 300.0f, -20.0f, 1.0f);
	//glm::vec4 lightPos = glm::vec4(0.0f, 0.0f, 80.0f, 1.0f); // top light
	glm::vec3 lightIntensity = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 lightAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 lightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 lightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

	// Send values to shader-program
	dir = glGetUniformLocation(shaderID, "matAmbient");
	if (dir >= 0){
		glUniform3fv(dir, 1, (float*)&matAmbient);
	}
	dir = glGetUniformLocation(shaderID, "matDiffuse");
	if (dir >= 0){
		glUniform3fv(dir, 1, (float*)&matDiffuse);
	}
	dir = glGetUniformLocation(shaderID, "matSpecular");
	if (dir >= 0){
		glUniform3fv(dir, 1, (float*)&matSpecular);
	}
	dir = glGetUniformLocation(shaderID, "lightIntensity");
	if (dir >= 0){
		glUniform3fv(dir, 1, (float*)&lightIntensity);
	}
	dir = glGetUniformLocation(shaderID, "lightAmbient");
	if (dir >= 0){
		glUniform3fv(dir, 1, (float*)&lightAmbient);
	}
	dir = glGetUniformLocation(shaderID, "lightDiffuse");
	if (dir >= 0){
		glUniform3fv(dir, 1, (float*)&lightDiffuse);
	}
	dir = glGetUniformLocation(shaderID, "lightSpecular");
	if (dir >= 0){
		glUniform3fv(dir, 1, (float*)&lightSpecular);
	}	
	dir = glGetUniformLocation(shaderID, "lightPos");
	if (dir >= 0){
		glUniform4fv(dir, 1, (float*)&lightPos);
	}	
	dir = glGetUniformLocation(shaderID, "matShiny");
	if (dir >= 0){
		glUniform1f(dir, matShiny);
	}

	// Draw the specified object using the SubDivision algorithm
	bezierSubDivision(4, "./teapot.data");

	// Sampling algorithm data for The Klein Bottle
	glm::vec3 (*klein_f[4]) (float u, float v);
    glm::vec3 (*klein_n[4]) (float u, float v);
	klein_f[0] = klein_bot;
    klein_n[0] = klein_bot_n;
	klein_f[1] = klein_handle;
    klein_n[1] = klein_handle_n;
    klein_f[2] = klein_top;
    klein_n[2] = klein_top_n;
	klein_f[3] = klein_middle;
    klein_n[3] = klein_middle_n;

	// Sampling algorithm data for Dini's Surface
    glm::vec3 (*dini_fs[4]) (float u, float v);
    glm::vec3 (*dini_ds[4]) (float u, float v);
    dini_fs[0] = dini_d;
    dini_ds[0] = dini_n;

	// Draw the specified object using the Sampling algorithm
	//generalSampling(klein_f, klein_n, 0.0f, M_PI*2.0f, 0.0f, M_PI, 50.0f, 50.0f, 4); // Klein Bottle
	//generalSampling(dini_fs, dini_ds, 0.0f, M_PI*6.0f, 0.001f, 2.0f, 50.0f, 50.0f, 1); // Dini's Surface
	
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

