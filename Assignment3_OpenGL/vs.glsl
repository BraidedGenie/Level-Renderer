#version 330 // GLSL 3.30

struct ATTRIBUTES{
	vec3 Kd; 
		float d;
	vec3 Ks; 
		float Ns;
	vec3 Ka; 
		float sharpness;
	vec3 Tf; 
		float Ni;
	vec3 Ke; 
		uint illum;
};

layout(std140, row_major) uniform ubo_data
{
	vec4 viewPos;
	vec4 lightColor, lightDir;
	mat4 world, view, proj, viewprojMatrix;
	ATTRIBUTES material;
};

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 uv;
layout (location = 2) in vec3 norm;

out vec3 world_nrm;

void main()
{
	gl_Position = vec4(pos, 1) * world * viewprojMatrix;
	world_nrm = norm;
}