#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in mat4 model;
layout(location=7) in vec4 albedo;
layout(location=8) in vec4 surface;
uniform mat4 vp,lightVP;
uniform float time;
out vec3 world,normal,local;
out vec2 uv;
flat out vec4 color,mat;
out vec4 shadowCoord;
void main(){
 vec3 p=aPos,n=aNormal;
 if(surface.z>5.5&&surface.z<6.5){
  float z=p.z; float wave=sin(time*1.12-abs(z)*.66)*pow(abs(z)*.25,1.7)*.68;
  p.y+=wave;n=normalize(vec3(n.x,n.y,n.z-cos(time*1.12-abs(z)*.66)*sign(z)*pow(abs(z)*.25,1.5)*.28));
 }
 if(surface.z>10.5&&surface.z<11.5){p.y+=sin(time*1.65+p.x*2.4)*abs(p.x)*.09;p.z+=sin(time*1.30+p.x*1.8)*abs(p.x)*.05;}
 vec4 w=model*vec4(p,1);world=w.xyz;local=p;
 normal=normalize(transpose(inverse(mat3(model)))*n);uv=aUV;color=albedo;mat=surface;
 shadowCoord=lightVP*w;gl_Position=vp*w;
}
