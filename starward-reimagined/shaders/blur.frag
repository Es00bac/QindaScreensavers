#version 330 core
in vec2 uv;out vec4 frag;uniform sampler2D image;uniform vec2 direction;uniform int extract;
vec3 sampleColor(vec2 p){vec3 c=texture(image,p).rgb;if(extract==1)c*=max(0,max(c.r,max(c.g,c.b))-.95)/max(max(c.r,max(c.g,c.b)),.0001);return c;}
void main(){vec3 c=sampleColor(uv)*.227027;c+=sampleColor(uv+direction*1.384615)*.316216;c+=sampleColor(uv-direction*1.384615)*.316216;c+=sampleColor(uv+direction*3.230769)*.070270;c+=sampleColor(uv-direction*3.230769)*.070270;frag=vec4(c,1);}
