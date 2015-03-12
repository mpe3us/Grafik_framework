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

uniform mat4 uModelMatrix;
uniform mat3 normalvectorMatrix;
uniform mat4 projectionMatrix;

out vec4 curPixel;
out vec3 curNormalVec;

void main() {
    curPixel = uModelMatrix * vec4(vertPosition, 1.0f);
    curNormalVec = normalize(normalvectorMatrix * vertReflect);

    gl_Position = projectionMatrix * uModelMatrix * vec4(vertPosition.xyz, 1.0);
}


