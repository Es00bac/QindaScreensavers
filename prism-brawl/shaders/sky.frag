#version 330 core
in vec2 uv;out vec4 frag;
uniform sampler2D noiseTex;uniform vec2 resolution;uniform float time,fov;uniform int chapter;uniform vec3 eye,target,up;uniform float seed;
const float PI=3.14159265;
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float n3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3-2*f);vec2 u=i.xy+vec2(37,17)*i.z+f.xy;return mix(texture(noiseTex,(u+.5)/256.).r,texture(noiseTex,(u+vec2(37,17)+.5)/256.).r,f.z);}
float fbm(vec3 p){float r=0,a=.54;for(int i=0;i<5;i++){r+=a*n3(p);p=p*2.04+vec3(17,9,21);a*=.5;}return r;}
vec3 stars(vec2 p,float scale){vec2 cell=floor(p*scale),pos=fract(p*scale);vec2 off=vec2(hash(cell),hash(cell+28.2));float r=length(pos-off);float bright=pow(hash(cell+81.2),17.);return mix(vec3(.35,.62,1),vec3(1,.64,.48),hash(cell+12))*bright*exp(-r*r*2600.)*1.5;}
vec3 moon(vec3 col,vec3 ray,vec3 center,float radius,vec3 tint,bool eclipse){
 vec3 oc=eye-center;float b=dot(oc,ray),c=dot(oc,oc)-radius*radius,disc=b*b-c;
 if(disc>0&&b<0){
  vec3 p=eye+ray*(-b-sqrt(disc)),n=normalize(p-center);
  float light=max(dot(n,normalize(vec3(-.68,.40,.64))),0.);
  float grain=fbm(n*9.0)*.38+.62;
  float rim=pow(1-max(dot(n,-ray),0.),4.);
  col=eclipse?vec3(.004,.009,.014)+tint*rim*.34:tint*(.065+light*.62)*grain+tint*rim*.42;
 }
 float alignment=dot(ray,normalize(center-eye));
 float edge=radius/length(center-eye);
 float glow=exp(-abs(sqrt(max(0.,1-alignment*alignment))-edge)*190.);
 return col+tint*glow*(eclipse?.10:.035);
}
void main(){
 vec2 q=(uv-.5)*2.;q.x*=resolution.x/resolution.y;
 vec3 F=normalize(target-eye),R=normalize(cross(F,up)),U=cross(R,F);
 vec3 ray=normalize(F+R*q.x*tan(fov*.5)+U*q.y*tan(fov*.5));
 vec2 sp=vec2(atan(ray.z,ray.x)/(2*PI),asin(ray.y)/PI);
 float elevation=clamp(ray.y*.8+.32,0.,1.);
 float haze=exp(-abs(ray.y+.055)*5.0);
 float cloud=fbm(ray*4.4+vec3(seed*.013+time*.0014,0,0));
 vec3 col;
 if(chapter==3){
  // Qinda Bliss: warm daylight, rolling cumulus, an open blue horizon.
  col=mix(vec3(.56,.68,.58),vec3(.025,.18,.52),smoothstep(-.36,.15,ray.y));
  vec3 sun=normalize(vec3(.40,.055,-1));float glow=pow(max(dot(ray,sun),0.),45.);
  col+=vec3(.56,.30,.095)*glow;
  float clouds=smoothstep(.43,.62,fbm(ray*6.7+vec3(time*.003,0,0)))*smoothstep(-.30,.06,ray.y);
  col=mix(col,mix(vec3(.51,.58,.64),vec3(.91,.86,.69),cloud),clouds*.78);
  col+=vec3(1,.81,.40)*pow(max(dot(ray,sun),0.),2400.)*.7;
 }else if(chapter==6){
  // Blue mineral light leaves the sculptural folds to the real 3D geometry.
  col=mix(vec3(.20,.37,.65),vec3(.012,.082,.32),smoothstep(-.52,.16,ray.y));
  col+=vec3(.12,.17,.22)*pow(max(dot(ray,normalize(vec3(-.55,.42,-1))),0.),18.);
  col+=vec3(.075,.045,.10)*cloud*haze;
 }else if(chapter==4){
  // The workshop has an enclosed, ink-and-apricot atmosphere, not outer space.
  col=mix(vec3(.022,.029,.031),vec3(.006,.012,.018),elevation);
  col+=vec3(.065,.024,.008)*pow(max(dot(ray,normalize(vec3(-.45,.12,-1))),0.),10.);
 }else if(chapter==2){
  col=mix(vec3(.019,.042,.087),vec3(.005,.012,.033),elevation);
  float clouds=fbm(ray*8.0+vec3(time*.002,0,0));
  col+=vec3(.025,.039,.068)*smoothstep(.35,.72,clouds);
  col+=vec3(.018,.015,.045)*haze;
  col=moon(col,ray,vec3(-175,118,-610),48.,vec3(.33,.43,.64),false);
 }else if(chapter==1){
  col=mix(vec3(.018,.064,.057),vec3(.004,.011,.028),elevation);
  col+=vec3(.026,.042,.039)*cloud*haze;
  col+=stars(sp,145.)*.23;
  col=moon(col,ray,vec3(-170,125,-600),63.,vec3(.35,.53,.49),false);
 }else if(chapter==5){
  col=mix(vec3(.035,.078,.12),vec3(.004,.009,.031),elevation);
  col+=stars(sp,140.)+stars(sp+2.,265.)*.5;
  col=moon(col,ray,vec3(160,65,-570),125.,vec3(.24,.49,.69),false);
  float azimuth=atan(ray.x,-ray.z);
  // Layered curtains move slowly across the sky, with fine vertical pleats.
  for(int curtain=0;curtain<3;++curtain){float offset=float(curtain)*.14;
   float base=.08+offset+.095*sin(azimuth*3.4+time*.018+offset*7.);
   float height=ray.y-base;
   float band=exp(-pow(height*8.5,2.))*smoothstep(-.045,.035,height);
   float pleats=.48+.52*pow(.5+.5*sin(azimuth*87.+cloud*4.+offset*8.),3.);
   vec3 color=mix(vec3(.025,.36,.22),vec3(.16,.10,.36),clamp(height*4.+offset,0.,1.));
   col+=color*band*pleats*.62;
  }
  col+=vec3(.025,.095,.12)*haze*cloud;
 }else{
  // The dark Pengu/Ducke wallpaper: obsidian, a jade horizon and an eclipse.
  col=mix(vec3(.010,.023,.031),vec3(.002,.006,.012),elevation);
  col+=vec3(.005,.027,.026)*haze*cloud;
  col+=stars(sp,145.)*.25;
  col=moon(col,ray,vec3(175,60,-610),158.,vec3(.20,.39,.42),true);
 }
 frag=vec4(col,1);
}
