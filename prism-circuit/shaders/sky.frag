#version 330 core
in vec2 uv;out vec4 frag;
uniform sampler2D noiseTex;uniform vec2 resolution;uniform float time,fov;uniform int chapter;uniform vec3 eye,target,up;uniform float seed;
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float n3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3-2*f);vec2 u=i.xy+vec2(37,17)*i.z+f.xy;return mix(texture(noiseTex,(u+.5)/256.).r,texture(noiseTex,(u+vec2(37,17)+.5)/256.).r,f.z);}
float fbm(vec3 p){float r=0,a=.54;for(int i=0;i<5;i++){r+=a*n3(p);p=p*2.04+vec3(17,9,21);a*=.5;}return r;}
vec3 stars(vec2 p,float scale){vec2 cell=floor(p*scale),pos=fract(p*scale);vec2 off=vec2(hash(cell),hash(cell+28.2));float r=length(pos-off);float bright=pow(hash(cell+81.2),17.);return mix(vec3(.35,.62,1),vec3(1,.64,.48),hash(cell+12))*bright*exp(-r*r*2600.)*1.5;}
void main(){
 vec2 q=(uv-.5)*2.;q.x*=resolution.x/resolution.y;
 vec3 F=normalize(target-eye),R=normalize(cross(F,up)),U=cross(R,F);
 vec3 ray=normalize(F+R*q.x*tan(fov*.5)+U*q.y*tan(fov*.5));
 vec2 sp=vec2(atan(ray.z,ray.x)/6.28318,asin(ray.y)/3.14159);
 float cloud=fbm(ray*3.6+vec3(seed*.009,0,0)),veins=fbm(ray*9.8+cloud*2.5);
 float ribbon=exp(-pow((ray.y+.23+sin(atan(ray.z,ray.x)*2.)*.11)*4.1,2.));
 vec3 gas=mix(vec3(.025,.055,.15),vec3(.12,.018,.15),smoothstep(.40,.63,veins));
 vec3 col=vec3(.002,.005,.015)+gas*pow(cloud,2.)*ribbon*1.0;
 col+=vec3(.012,.11,.10)*pow(veins,4.)*.5;
 col+=stars(sp,140.)+stars(sp+2.,260.)*.65;
 // A slowly striated gas giant and ring system, fixed in world space.
 vec3 center=vec3(-45,-154,-430);float radius=128.;vec3 oc=eye-center;
 float b=dot(oc,ray),c=dot(oc,oc)-radius*radius,disc=b*b-c;
 float planetT=1e20;
 if(disc>0&&b<0){
  planetT=-b-sqrt(disc);vec3 p=eye+ray*planetT,n=normalize(p-center);
  float bands=.5+.5*sin(n.y*39.+fbm(n*8.)*4.5);
  vec3 surface=mix(vec3(.10,.17,.31),vec3(.32,.19,.33),bands);
  float light=max(dot(n,normalize(vec3(-.3,.7,.6))),0.);
  col=surface*(.12+1.9*light)+vec3(.10,.22,.40)*pow(1-max(dot(n,-ray),0),3.)*.9;
 }
 vec3 rn=normalize(vec3(.08,1,.32));float den=dot(ray,rn);
 if(abs(den)>.001){float t=dot(center-eye,rn)/den;if(t>0&&t<planetT){
  float d=length(eye+ray*t-center);
  if(d>148&&d<207){float ring=.12+.10*pow(.5+.5*sin(d*2.7),3.);col=mix(col,vec3(.19,.14,.32)+vec3(.20,.32,.42)*ring,smoothstep(148,152,d)*(1-smoothstep(202,207,d))*.67);}
 }}
 // A broad, dim aurora, without strobing or screen-space swirl effects.
 float aurora=pow(.5+.5*sin(ray.y*24.+fbm(ray*2.+vec3(time*.003,0,0))*10.),9.)*smoothstep(.0,.8,ray.y);
 col+=vec3(.016,.13,.10)*aurora*.3;
 frag=vec4(col,1);
}
