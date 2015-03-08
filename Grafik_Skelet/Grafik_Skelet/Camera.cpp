#include "Camera.h"

Camera::Camera(glm::vec3 const& vrp, 
			   glm::vec3 const& vpn, 
			   glm::vec3 const& vup, 
			   glm::vec3 const& prp,
			   glm::vec2 const& lower_left_window, 
			   glm::vec2 const& upper_right_window,
			   float front_plane, 
			   float back_plane,
			   int window_width, 
			   int window_height)
{
	this->vrp = vrp;
	this->vpn = vpn;
	this->vup = vup;
	this->prp = prp;
	this->lower_left_window = lower_left_window;
	this->upper_right_window = upper_right_window;
	this->front_plane = front_plane;
	this->back_plane = back_plane;
	this->window_width = window_width;
	this->window_height = window_height;

	// Init camera methods
	this->ComputeViewOrientation(this->vrp, this->vpn, this->vup);
	this->ComputeViewProjection(this->prp,	this->lower_left_window, this->upper_right_window, this->front_plane, this->back_plane);
	this->ComputeWindowViewport(this->window_width, this->window_height);

	// Set current transformation matrix
	this->currenttransformationmatrix =  /* this->windowviewportmatrix * */ this->MperparMatrix * (this->viewprojectionmatrix * this->Rmatrix * this->Tmatrix);
	this->invcurrenttransformationmatrix = glm::inverse(this->currenttransformationmatrix);
}

// Compute the View Orientation Matrix
void Camera::ComputeViewOrientation(glm::vec3& vrp, glm::vec3& vpn, glm::vec3& vup)
{
	// Init the R-matrix
	glm::mat4 R(1.0f);
	// Compute rotations
	glm::vec3 Rz = glm::normalize(vpn);
	glm::vec3 Rx = glm::normalize(glm::cross(vup, Rz));
	glm::vec3 Ry = glm::normalize(glm::cross(Rz, Rx));
	// Insert them in rows
	glm::vec4 row1 = glm::vec4(Rx, 0.0f);
	glm::vec4 row2 = glm::vec4(Ry, 0.0f);
	glm::vec4 row3 = glm::vec4(Rz, 0.0f);
	glm::vec4 row4 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	// Insert the rows in the R-matrix
	R[0][0] = row1.x; R[1][0] = row1.y; R[2][0] = row1.z; R[3][0] = row1.w;
	R[0][1] = row2.x; R[1][1] = row2.y; R[2][1] = row2.z; R[3][1] = row2.w;
	R[0][2] = row3.x; R[1][2] = row3.y; R[2][2] = row3.z; R[3][2] = row3.w;
	R[0][3] = row4.x; R[1][3] = row4.y; R[2][3] = row4.z; R[3][3] = row4.w;

	// Compute the Tvrp-matrix
	glm::mat4 Tvrp = glm::translate(-vrp);	
	
	this->Rmatrix = R;
	this->Tmatrix = Tvrp;
	
	// Combined
	this->vieworientationmatrix = R * Tvrp;
	this->invvieworientationmatrix = glm::inverse(this->vieworientationmatrix);
}

// Compute the Projection Orientation Matrix
void Camera::ComputeViewProjection(glm::vec3& prp,
	glm::vec2& lower_left_window, glm::vec2& upper_right_window,
	float front_clipping_plane, float back_clipping_plane)
{
	// Compute the Tprp-matrix
	glm::mat4 Tprp = glm::translate(-prp);	

	// Compute CW
	glm::vec3 CW = glm::vec3((lower_left_window.x + upper_right_window.x) / 2.0f, 
    (lower_left_window.y + upper_right_window.y) / 2.0f,
    0.0f);
	// Compute DOP
	glm::vec4 DOP = glm::vec4(prp - CW, 0.0f);

	// Init the Sh matrix
	glm::mat4 Sh(1.0f);
	float shx = - DOP.x / DOP.z;
	float shy = - DOP.y / DOP.z;
	// Insert shx and shy into the Sh matrix
	Sh[2][0] = shx;
	Sh[2][1] = shy;

	// Compute VRP mark
	glm::vec4 vec = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 VRP_mark = Sh * Tprp * vec; 

	// Compute the scale factors for Sper
	float sx = ( -2.0f * prp.z ) / ((upper_right_window.x - lower_left_window.x) * (back_clipping_plane - prp.z));
	float sy = ( -2.0f * prp.z ) / ((upper_right_window.y - lower_left_window.y) * (back_clipping_plane - prp.z));
	float sz = -1.0f / (back_clipping_plane - prp.z);

	// Init the Sper-matrix
	glm::mat4 Sper(1.0f);	
	// Insert -VRP into the matrix
	Sper[0][0] = sx;
	Sper[1][1] = sy;
	Sper[2][2] = sz;

	// Compute the scaling changes
	float Zmin = -1.0f;
	float Zmax = (front_clipping_plane - prp.z) / (back_clipping_plane - prp.z);
	float Zp = prp.z / (back_clipping_plane - prp.z);

	// Init the Mperpar-matrix
	glm::mat4 Mperpar(1.0f);	
	// Insert values into the matrix
	Mperpar[2][2] = 1.0f / (1.0f+Zmax);
	Mperpar[2][3] = -1.0f;
	Mperpar[3][2] = -Zmax / (1.0f+Zmax);

	this->MperparMatrix = Mperpar;

	// Combined
	this->viewprojectionmatrix = Sper * Sh * Tprp;
	this->invviewprojectionmatrix = glm::inverse(this->viewprojectionmatrix );
}

// Compute the Window Viewport Matrix
void Camera::ComputeWindowViewport(int window_width, int window_height)
{
	// Init the S-matrix
	glm::mat4 S(1.0f);	
	// Insert values into the matrix
	S[0][0] = window_width/2.0f;
	S[1][1] = window_height/2.0f;;
	S[2][2] = 1.0f;
	S[3][3] = 1.0f;

	// Init the T-matrix
	glm::mat4 T(1.0f);	
	// Insert values into the matrix
	T[3][0] = 1.0f;
	T[3][1] = 1.0f;

	// Compute the Mperpar-matrix
	glm::mat4 Mwv = S * T;

	// Combined
	this->windowviewportmatrix = Mwv;
	this->invwindowviewportmatrix = glm::inverse(this->windowviewportmatrix);
}

glm::mat4x4 Camera::ViewOrientation() { return this->vieworientationmatrix; }
glm::mat4x4 Camera::InvViewOrientation() { return this->invvieworientationmatrix;  }
glm::mat4x4 Camera::ViewProjection() { return this->viewprojectionmatrix; }
glm::mat4x4 Camera::InvViewProjection() { return this->invviewprojectionmatrix; }
glm::mat4x4 Camera::WindowViewport() { return this->windowviewportmatrix; }
glm::mat4x4 Camera::InvWindowViewport() { return this->invwindowviewportmatrix; }
glm::mat4x4 Camera::CurrentTransformationMatrix() { return this->currenttransformationmatrix; }
glm::mat4x4 Camera::InvCurrentTransformationMatrix() { return this->invcurrenttransformationmatrix; }

glm::vec3 const& Camera::VRP() const { return this->vrp; }
void Camera::VRP(glm::vec3 const& vrp) { this->vrp=vrp; }
glm::vec3 const& Camera::VPN() const { return this->vpn; }
void Camera::VPN(glm::vec3 const& vpn) { this->vpn=vpn; }
glm::vec3 const& Camera::VUP() const { return this->vup; }
void Camera::VUP(glm::vec3 const& vup) { this->vup=vup; }
glm::vec3 const& Camera::PRP() const { return this->prp; }
void Camera::PRP(glm::vec3 const& prp) { this->prp=prp; }

glm::vec2 const& Camera::WinLowerLeft() const { return this->lower_left_window; }
void Camera::WinLowerLeft(glm::vec2 const& lower_left_window) { this->lower_left_window = lower_left_window; }
glm::vec2 const& Camera::WinUpperRight() const { return this->upper_right_window; };
void Camera::WinUpperRight(glm::vec2 const& upper_right_window) { this->upper_right_window = upper_right_window; };

float Camera::FrontClippingPlane() const { return this->front_plane; }
void Camera::FrontClippingPlane(float const front_plane) { this->front_plane=front_plane; }
float Camera::BackClippingPlane() const { return this->back_plane; }
void Camera::BackClippingPlane(float const back_plane) { this->back_plane=back_plane; }

int Camera::WindowWidth() { return window_width; }
void Camera::WindowWidth(int new_window_width){ this->window_width = new_window_width; }
int Camera::WindowHeight(){  return this->window_height;}
void Camera::WindowHeight(int new_window_height){  this->window_height = new_window_height;}

Camera::Camera(void)
{
}


Camera::~Camera(void)
{
}
