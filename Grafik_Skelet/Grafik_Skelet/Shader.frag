#version 330

layout(location = 0) out vec4 colourOut;

// Light variables
uniform vec4 lightPos;
uniform vec3 lightIntensity;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

// Material variables
uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matShiny;

// Matrices
uniform mat4 uModelMatrix;
uniform mat3 normalvectorMatrix;
uniform mat4 projectionMatrix;

in vec4 curVert;
in vec3 curNormalVec;

// Function for Phong's Reflection Model 
vec3 phongReflection() {
	vec4 lightPosTransformation = uModelMatrix * lightPos;
	vec3 n = normalize( curNormalVec );
	vec3 s = normalize( vec3(lightPosTransformation - curVert) );
	vec3 v = normalize( vec3(-curVert) );
	vec3 r = reflect( -s, n );
	return (matAmbient * lightAmbient + 
	        matDiffuse * lightDiffuse * max( dot(s,n), 0.0 ) +
			    matSpecular * lightSpecular * pow(max(dot(r,v), 0.0), matShiny));
}

void main() {
   colourOut = vec4(phongReflection(), 1.0);
   //colourOut = vec4(1.0, 0.0, 0.0, 1.0);


}
