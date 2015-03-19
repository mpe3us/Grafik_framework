#version 330

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertReflect;

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

out vec4 curVert;
out vec3 curNormalVec;

void main() {
    // Compute position of the current vertex
    curVert = uModelMatrix * vec4(vertPosition, 1.0f);
    // Compute the current normal vector
    curNormalVec = normalize(normalvectorMatrix * vertReflect);

    gl_Position = projectionMatrix * uModelMatrix * vec4(vertPosition.xyz, 1.0);
}


