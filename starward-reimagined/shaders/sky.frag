#version 330 core
in vec2 uv;out vec4 frag;
uniform sampler2D noiseTex;uniform vec2 resolution;uniform float time;uniform int chapter;uniform vec3 eye,target;uniform float seed,fov;
float noise(vec2 p){return texture(noiseTex,p*.035/256.0).r;}
float fbm(vec2 p){float a=.54,r=0;for(int i=0;i<6;i++){r+=a*noise(p);p=mat2(.80,-.60,.60,.80)*p*2.04+17.3;a*=.50;}return r;}
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
vec3 stars(vec2 p,float scale){vec2 c=floor(p*scale),f=fract(p*scale);vec2 offset=vec2(hash(c),hash(c+31.7));vec2 d=(f-offset)*scale;float lum=pow(hash(c+77.1),19);float dist=length(f-offset);float core=exp(-dist*dist*4200.0);float rays=exp(-abs(f.x-offset.x)*700)*exp(-abs(f.y-offset.y)*100)+exp(-abs(f.y-offset.y)*700)*exp(-abs(f.x-offset.x)*100);float tw=.94+.06*sin(time*.30+hash(c)*18);return mix(vec3(.49,.68,1),vec3(1,.76,.51),hash(c+12))*lum*(core*2.5+rays*.10)*tw;}
float value3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3-2*f);vec2 uv=i.xy+vec2(37,17)*i.z+f.xy;return mix(texture(noiseTex,(uv+.5)/256.0).r,texture(noiseTex,(uv+vec2(37,17)+.5)/256.0).r,f.z);}
float f3(vec3 p){float r=0,a=.53;for(int i=0;i<5;i++){r+=value3(p)*a;p=p*2.07+vec3(31.4,18.5,7.6);a*=.5;}return r;}
void main(){
 vec2 q=uv-.5;q.x*=resolution.x/resolution.y;
 vec2 p=q*2.2+vec2(seed*.37,seed*.19);
 vec2 warp=vec2(fbm(p*35),fbm(p*35+32));float neb=fbm(p*95+warp*63);
 float ridge=pow(clamp(1-abs(neb-.57)*3.1,0,1),4);
 float band=exp(-pow((q.y+q.x*.30-.10)*3.1,2));float dust=smoothstep(.41,.66,fbm(p*120+warp*100));
 vec3 cyan=vec3(.012,.090,.125),orange=vec3(.23,.09,.024),violet=vec3(.084,.036,.17);
 if(chapter==1){cyan=vec3(.030,.066,.09);orange=vec3(.28,.11,.026);}
 if(chapter==3||chapter==4||chapter==5){cyan=vec3(.021,.082,.14);orange=vec3(.11,.035,.22);}
 vec3 gas=mix(cyan,orange,clamp(fbm(p*24)*1.5-.25,0,1));
 vec3 col=vec3(.003,.007,.015)+gas*ridge*band*(.45+.7*dust)+violet*pow(neb,4)*.21;
 col*=.48+.52*smoothstep(.29,.59,fbm(p*65+warp*40));
 col+=stars(q+vec2(time*.000025,0),95)+stars(q+vec2(.3,.2),177)*.7+stars(q-1.6,48);
 // A distant warm sun has a restrained diffraction streak, not a screen-filling flash.
 vec2 sun=q-vec2(-.66,.30);float d=length(sun);col+=vec3(1,.55,.21)*(.028/(1+d*d*120)+.09*exp(-d*30));col+=vec3(.40,.18,.045)*exp(-abs(sun.y)*210)*exp(-abs(sun.x)*4)*.2;
 // Ray/sphere planet with layered ocean/continent/cloud fields and twilight atmosphere.
 vec3 F=normalize(target-eye),R=normalize(cross(F,vec3(0,1,0))),U=cross(R,F);vec3 ray=normalize(F+R*q.x*2*tan(fov*.5)+U*q.y*2*tan(fov*.5));
 vec3 center;float radius;
 if(chapter==0||chapter==6){center=vec3(-27,-27,-59);radius=27;}
 else if(chapter==1||chapter==2){center=vec3(15,14,-80);radius=15;}
 else if(chapter==5){center=vec3(-50,-37,-100);radius=35;}else{center=vec3(-33,-24,-98);radius=22;}
 vec3 oc=eye-center;float b=dot(oc,ray),c=dot(oc,oc)-radius*radius,disc=b*b-c;
 float nearEdge=max(0.0,length(cross(center-eye,ray))-radius);
 col+=vec3(.06,.24,.5)*exp(-nearEdge*1.8)*step(0,-b)*.23;
 float planetDistance=disc>0&&b<0?-b-sqrt(disc):1e10;
 if(disc>0&&b<0){vec3 pos=eye+ray*(-b-sqrt(disc)),n=normalize(pos-center);float daylight=dot(n,normalize(vec3(-.68,.49,.45)));vec3 nrot=vec3(n.x*cos(time*.0004)+n.z*sin(time*.0004),n.y,-n.x*sin(time*.0004)+n.z*cos(time*.0004));
  float land=f3(nrot*4.5);float cloud=f3(nrot*8.5+vec3(time*.0006,12.0,0));float details=f3(nrot*63);
  vec3 sea=vec3(.009,.044,.097);vec3 continent=mix(vec3(.038,.071,.055),vec3(.20,.14,.085),details);
  vec3 surface=mix(sea,continent,smoothstep(.48,.56,land));surface=mix(surface,vec3(.69,.76,.80),smoothstep(.56,.69,cloud)*.80);
  if(chapter>=3&&chapter<=5)surface=mix(vec3(.09,.025,.18),vec3(.18,.36,.49),f3(nrot*5.5));
  float nd=max(daylight,0);vec3 planet=surface*(.036+nd*2.4);
  float cities=pow(clamp((details-.54)*6,0,1),4)*smoothstep(.48,.56,land)*(1-smoothstep(-.15,.12,daylight));planet+=vec3(1,.48,.14)*cities*.18*float(chapter<3||chapter==6);
  float rim=pow(1-max(dot(n,-ray),0),3);planet+=vec3(.075,.22,.51)*rim*pow(clamp(daylight+.5,0,1),.6)*1.25;
  col=planet;
 }
 // A tilted, dusty ring system with a fine Cassini gap. The front and back
 // are depth-tested analytically against the planet, including its cast shadow.
 if(chapter==2||chapter==3||chapter==5){
  vec3 N=normalize(vec3(.18,.85,.49));float denom=dot(ray,N);
  if(abs(denom)>.001){float rt=dot(center-eye,N)/denom;
   vec3 rp=eye+ray*rt-center;float rr=length(rp)/radius;
   if(rt>0&&rt<planetDistance&&rr>1.25&&rr<2.05){
    float bands=.42+.28*sin(rr*142)+.12*sin(rr*381);float gap=1-smoothstep(.009,.028,abs(rr-1.65));
    float edge=smoothstep(1.25,1.31,rr)*(1-smoothstep(1.99,2.05,rr));float opacity=edge*(.35+bands*.34)*(1-gap*.91);
    vec3 light=normalize(vec3(-.68,.49,.45));float sh=dot(rp,light);float shadow=sh<0&&dot(rp,rp)-sh*sh<radius*radius?.19:1.;
    vec3 ring=mix(vec3(.19,.25,.36),vec3(.67,.48,.29),bands)*shadow;
    col=mix(col,ring,opacity);
   }
  }
 }
 // A slender auroral curtain on the far side of the alien chapters.
 if(chapter>=3&&chapter<=5){float curtain=exp(-abs(q.y-.30-.07*sin(q.x*5+time*.018))*46);float ribs=pow(.5+.5*sin(q.x*92+fbm(p*90)*9),3);col+=vec3(.012,.10,.11)*curtain*(.3+ribs*.7);}
 frag=vec4(col*.88,1);
}
