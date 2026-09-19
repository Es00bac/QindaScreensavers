#version 330 core
in vec2 uv;out vec4 frag;uniform sampler2D image,bloom,overlay;uniform vec2 resolution;uniform float fade,brightness,bloomAmount;
vec3 aces(vec3 c){return clamp((c*(2.51*c+.03))/(c*(2.43*c+.59)+.14),0,1);}
vec3 colorAt(vec2 p){return texture(image,p).rgb;}
void main(){
 vec2 px=1/resolution;vec3 c=colorAt(uv);vec3 n=colorAt(uv+vec2(0,px.y)),s=colorAt(uv-vec2(0,px.y)),e=colorAt(uv+vec2(px.x,0)),w=colorAt(uv-vec2(px.x,0));
 vec3 L=vec3(.299,.587,.114);float center=dot(c,L),lo=min(center,min(min(dot(n,L),dot(s,L)),min(dot(e,L),dot(w,L)))),hi=max(center,max(max(dot(n,L),dot(s,L)),max(dot(e,L),dot(w,L))));
 // A compact edge-sensitive antialias pass preserves small hull lines and lettering.
 float blend=clamp((hi-lo)/max(hi,.10)-.21,0,.55);c=mix(c,(n+s+e+w)*.25,blend);
 c+=texture(bloom,uv).rgb*bloomAmount;
 c=aces(c*1.1);c=pow(c,vec3(1/2.2));
 float vignette=1-.18*pow(length((uv-.5)*vec2(1.2,1)),1.6);c*=vignette;
 vec4 text=texture(overlay,vec2(uv.x,1-uv.y));c=mix(c,text.rgb,text.a);
 frag=vec4(c*fade*brightness,1);
}
