#version 120

attribute vec4 aPosition;
attribute vec3 aNormal;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uLightPos;
uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform float Ushine;

varying vec3 vNormal, vLight, vView;

void main() {
  vec4 vPosition;
  vec4 light;
  vec3 Refl;
  vec3 Spec;
  vec3 Diffuse;
  vec3 tNorm;
  vec3 Half;

  /* First model transforms */
  vPosition = uModelMatrix*aPosition;
 
  //compute the normal in camera space 
  tNorm = vec3(uViewMatrix*uModelMatrix*vec4(normalize(aNormal), 0));

  //compute the point in camera space for specular
  vPosition = uViewMatrix*vPosition;
  //compute the light in camera 
  light = uViewMatrix*vec4(uLightPos, 1) - vPosition;
  light = normalize(light);
  
  //move the point into screen space
  gl_Position = uProjMatrix* vPosition;

   //compute the view vector
  vView = -1*vec3(vPosition);
  Refl = -1*vec3(light) + 2.0*(dot(normalize(tNorm),vec3(light)))*tNorm;
  vNormal = tNorm;
  vLight = vec3(light);
}
