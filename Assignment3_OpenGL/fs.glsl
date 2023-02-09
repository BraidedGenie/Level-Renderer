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

in vec3 world_nrm;

void main()
{
	//gl_FragColor = vec4(world_nrm, 1);
	float lightRatio = clamp(dot(-normalize(lightDir.xyz), world_nrm), 0, 1);
	vec3 direct = vec3(lightRatio) * material.Kd;
	vec3 viewdir = normalize(viewPos.xyz - world_nrm);
	vec3 halfvec = normalize((-lightDir.xyz)+ viewdir);
//	vec3 reflectdir = reflect(-lightDir.xyz, world_nrm);
//	float spec = pow(max(dot(viewdir, reflectdir), 0.0), 32);
	float intensity = max(pow(clamp(dot(viewdir, halfvec), 0, 1), material.Ns), 0);
	vec3 reflec = vec3(lightColor * intensity) * material.Ks;
	vec3 indirect = material.Ka;
	vec3 diffuse = material.Kd;
	vec3 emmisive = material.Ke;
	//vec3 specular = material.Ns * spec * material.Kd;

	vec4 result = vec4(clamp((indirect + direct) * diffuse + emmisive,0, 1), material.d);

	gl_FragColor = result;
}