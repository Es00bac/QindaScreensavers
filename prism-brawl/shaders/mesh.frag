#version 330 core
in vec3 world,normal,local;in vec2 uv;flat in vec4 color,mat;in vec4 shadowCoord;
uniform sampler2D noiseTex,shadowTex,hullTex;uniform vec3 eye,hero;uniform float time;uniform int chapter;
out vec4 frag;
const float PI=3.14159265;
float n3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3-2*f);vec2 uv=i.xy+vec2(37,17)*i.z+f.xy;return mix(texture(noiseTex,(uv+.5)/256.0).r,texture(noiseTex,(uv+vec2(37,17)+.5)/256.0).r,f.z);}
float fbm(vec3 p){return n3(p)*.54+n3(p*2.03)*.27+n3(p*4.07)*.135+n3(p*8.13)*.065;}
float shadow(){vec3 p=shadowCoord.xyz/shadowCoord.w*.5+.5;if(p.z>1.0||p.x<0||p.x>1||p.y<0||p.y>1)return 1.0;float s=0;float bias=max(.0018,.003*(1.0-dot(normalize(normal),normalize(vec3(-.5,.82,.65)))));for(int i=-1;i<=1;i++)for(int j=-1;j<=1;j++)s+=p.z-bias>texture(shadowTex,p.xy+vec2(i,j)/2048.0).r?.26:1.0;return s/9.0;}
vec3 light(vec3 N,vec3 V,vec3 L,vec3 radiance,vec3 base,float rough,float metal){
 vec3 H=normalize(V+L);float NV=max(dot(N,V),.001),NL=max(dot(N,L),.001),NH=max(dot(N,H),0),VH=max(dot(V,H),0);
 float a=rough*rough,a2=a*a,d=NH*NH*(a2-1)+1;float D=a2/(PI*d*d+.0001);float k=(rough+1)*(rough+1)/8;
 float G=NV/(NV*(1-k)+k)*NL/(NL*(1-k)+k);vec3 F=mix(vec3(.045),base,metal)+(1-mix(vec3(.045),base,metal))*pow(1-VH,5);
 return ((1-F)*(1-metal)*base/PI+D*G*F/max(4*NV*NL,.01))*radiance*NL;
}
void main(){
 if(mat.z>11.5 && mat.z<12.5){
  float r=length(local.xz),a=atan(local.z,local.x);float warp=sin(a*4.0-r*12.0+time*.27);
  float strand=pow(.5+.5*sin(a*7.0+pow(r,.6)*24.0-time*.55+warp),15.0);
  float edge=pow(clamp(r,0,1),14);vec3 pool=vec3(.002,.006,.021)+vec3(.035,.17,.32)*strand*r*(1-r)*4;
  pool+=vec3(.015,.25,.45)*edge*.55;pool+=vec3(.05,.024,.1)*pow(1-r,3);
  frag=vec4(pool*color.a,1);return;
 }
 vec3 N=normalize(normal),V=normalize(eye-world),base=color.rgb;
 // Holographic guards use a cut-out Fresnel shell. No opaque ball hides the pilot.
 if(mat.z>17.5 && mat.z<18.5){
  float rim=pow(1-abs(dot(N,V)),2.5);
  float scan=pow(.5+.5*sin(world.y*24.0),40.0);
  if(rim<.65 && scan<.97)discard;
  frag=vec4(color.rgb*(.20+rim*1.7)+vec3(.025)*scan,1);return;
 }
 if(mat.z>18.5 && mat.z<19.5){
  vec2 p=world.xz;vec2 cell=abs(fract(p*.5+.5)-.5);
  vec3 spectrum=.48+.43*cos(vec3(0,2.094,4.188)+p.x*.15+p.y*.25);
  float seam=smoothstep(.472,.49,max(cell.x,cell.y));
  base=mix(vec3(.028,.045,.070)+spectrum*.13,vec3(.004,.008,.018),seam);
 }

 float rough=mat.x,metal=mat.y,mode=mat.z;float emission=color.a;
 // Arena surfaces use their own material language. Fighter materials retain
 // their established palette; the room's light ties the entire scene together.
 if(mode>19.5&&mode<20.5){
  vec2 p=world.xz;vec2 cell=abs(fract(p*.24+.5)-.5);
  float seam=smoothstep(.477,.493,max(cell.x,cell.y));
  base*=.80+.20*fbm(world*.6);base=mix(base,base*.24,seam);
  float glint=pow(.5+.5*sin(p.x*.39+p.y*.61),28.);
  base+=vec3(.018,.050,.044)*glint;rough=.20;metal=.70;
 }else if(mode>20.5&&mode<21.5){
  vec2 p=world.xz,cell=fract(p*.64);float detail=n3(world*12.);
  float trace=(1-smoothstep(.014,.035,abs(cell.x-.24)))*step(.38,cell.y);
  trace=max(trace,(1-smoothstep(.014,.035,abs(cell.y-.38)))*step(.24,cell.x));
  base*=.68+.48*fbm(world*1.7)+detail*.055;
  vec3 ink=chapter==4?vec3(.31,.18,.08):vec3(.24,.37,.14);
  base=mix(base,ink,trace*.47);rough=chapter==4?.43:.77;metal=.17;
 }else if(mode>21.5&&mode<22.5){
  float puddle=smoothstep(.36,.63,fbm(world*.53));
  vec2 ripples=vec2(sin(world.x*3.4+time*.5),cos(world.z*4.8-time*.7));
  N=normalize(N+vec3(ripples.x,0,ripples.y)*.025*puddle);
  float reflection=pow(.5+.5*sin(world.x*1.6+sin(world.z*.5)*.2),24.);
  base*=.72; base+=mix(vec3(.013,.13,.22),vec3(.21,.025,.10),.5+.5*sin(world.x*.73))*reflection*puddle*.65;
  rough=mix(.40,.10,puddle);metal=.63;
 }else if(mode>22.5&&mode<23.5){
  if(!gl_FrontFacing)N=-N;
  float stratum=.5+.5*sin(local.x*7.+local.z*2.);
  base*=.87+.13*stratum;rough=.30;metal=.23;
  float pearl=pow(1-abs(dot(N,V)),3.);
  base+=vec3(.055,.10,.14)*pearl;
 }else if(mode>23.5&&mode<24.5){
  float noise=fbm(world*.7),fracture=pow(1-clamp(abs(noise-.49)*35.,0.,1.),4.);
  base*=.75+noise*.44; base+=vec3(.13,.25,.32)*fracture;
  base=mix(base,vec3(.55,.67,.73),smoothstep(.60,.77,noise)*.38);
  emission+=fracture*.18;rough=.20;metal=.31;
 }else if(mode>24.5&&mode<25.5){
  N=normalize(N+vec3(sin(world.x*.65+time*.21),0,cos(world.z*.82-time*.18))*.035);
  float wave=pow(.5+.5*sin(world.z*1.3+world.x*.18+time*.26),17.);
  float streak=pow(.5+.5*sin(world.x*.19+sin(world.z*.04)*.35),10.);
  base+=vec3(.02,.13,.13)*streak+vec3(.025,.047,.06)*wave;
  rough=.12;metal=.76;
 }else if(mode>25.5&&mode<26.5){
  vec2 grid=fract(uv*vec2(18,4));float slot=step(.18,grid.x)*step(grid.x,.8)*step(.28,grid.y)*step(grid.y,.65);
  base=mix(base,vec3(.016,.045,.040),slot);
  base+=vec3(.017,.10,.060)*slot;rough=.5;metal=.35;
 }else if(mode>26.5&&mode<27.5){
  float grain=fbm(world*.28),grass=n3(world*6.5);
  base*=.65+grain*.62+grass*.065;rough=.93;metal=.015;
 }else if(mode>27.5&&mode<28.5){
  if(!gl_FrontFacing)N=-N;
  float crest=pow(sin(uv.y*PI),3.);
  base*=.48+crest*.90;
  base+=vec3(.025,.16,.22)*pow(1-abs(dot(N,V)),3.);
  rough=.24;metal=.34;emission=.11;
 }
 if(mode>12.5&&mode<13.5){
  // Metallic prismatic pavers: dark grout, thin circuit traces, calm emission.
  float h=uv.y*.78+uv.x*.0017+.02*sin(uv.x*.024);
  vec3 spectrum=.48+.45*cos(6.28318*(h+vec3(0,-.333,.333)));
  float cell=fract(uv.x/2.4),crossCell=fract(uv.y*12.0);
  float aa=max(fwidth(cell),.003);
  float seam=1-smoothstep(.011,.011+aa,min(cell,1-cell));
  float across=1-smoothstep(.015,.027,min(crossCell,1-crossCell));
  base=spectrum*.12+vec3(.008,.012,.025);
  base*=1-.70*max(seam,across);
  float edge=smoothstep(.46,.494,abs(uv.y-.5));
  base=mix(base,spectrum*.48+vec3(.009),edge);
  float gridFine=pow(.5+.5*sin(uv.x*9.0),34.0)*.016;
  base+=spectrum*gridFine;
  emission=.26+edge*2.0;rough=.24;metal=.48;
 } else if(mode>13.5&&mode<14.5){
  vec2 grid=vec2(uv.x*14.,uv.y*35.);vec2 cell=fract(grid);
  float seed=fract(sin(dot(floor(grid),vec2(17.17,79.2)))*8193.3);
  float win=step(.28,cell.x)*step(cell.x,.73)*step(.22,cell.y)*step(cell.y,.64)*step(.45,seed);
  vec3 co=mix(vec3(.06,.36,.60),vec3(.59,.12,.39),step(.73,seed));
  base+=co*win*.19;emission=win*.6;rough=.33;metal=.5;
 } else if(mode>14.5&&mode<15.5){
  float tread=step(.24,fract(uv.x*26.+abs(uv.y-.5)*3.5));
  base*=.55+.45*tread;rough=.85;metal=.02;
 }

 if(mode>15.5 && mode<17.0){
  float row=floor((mode-16.0)*10.0+.25);
  vec4 decal=texture(hullTex,vec2(uv.x,(row+1.0-uv.y)/8.0));base=mix(base,decal.rgb,decal.a);
 }
 if((mode>1.5&&mode<2.5)||(mode>6.5&&mode<7.5)){
  float n=fbm(local*7.0);base*=.50+1.05*n;
  float e=.012;vec3 grad=vec3(fbm((local+vec3(e,0,0))*7)-n,fbm((local+vec3(0,e,0))*7)-n,fbm((local+vec3(0,0,e))*7)-n);
  N=normalize(N-grad*2.1);rough=.87;
  if(mode>6.5){float vein=pow(clamp(1-abs(n-.51)*32,0,1),5);base+=vec3(.32,.1,.007)*vein;emission+=vein*1.6;}
 }else if(mode>4.5&&mode<6.5){
  float n=fbm(local*4);base*=.72+.5*n;
  vec3 irid=.5+.5*cos(vec3(0,2,4)+dot(V,N)*7+local.y*.6);base+=irid*.04;
  if(mode>5.5){float rib=pow(.5+.5*sin(local.z*14+local.x*3),24);float edge=pow(abs(local.z)/4,4);base+=vec3(.012,.08,.1)*rib;emission+=rib*.2+edge*.40;}
 }else if(emission<.1){
  float speck=n3(local*40);base*=.96+speck*.08;
 }
 if(mode>8.5&&mode<9.5){vec4 decal=texture(hullTex,vec2(uv.x,1-uv.y));base=mix(base,decal.rgb,decal.a);rough+=decal.a*.15;}
 vec3 key=vec3(3.0,3.0,3.05),rim=vec3(.72,.60,1.35);
 if(chapter==0){key=vec3(2.5,2.8,2.85);rim=vec3(.27,.84,.72);}
 if(chapter==1){key=vec3(2.25,2.85,2.45);rim=vec3(.33,.74,.69);}
 if(chapter==2){key=vec3(2.35,2.55,3.2);rim=vec3(.97,.36,.94);}
 if(chapter==3){key=vec3(3.6,3.25,2.65);rim=vec3(.69,.90,1.10);}
 if(chapter==4){key=vec3(3.3,2.15,1.42);rim=vec3(.28,1.02,.75);}
 if(chapter==5){key=vec3(2.3,2.85,3.55);rim=vec3(.42,.85,1.35);}
 if(chapter==6){key=vec3(3.25,3.42,3.70);rim=vec3(.80,.73,1.30);}
 vec3 result=light(N,V,normalize(vec3(-.50,.82,.65)),key,base,rough,metal)*shadow();
 result+=light(N,V,normalize(vec3(.7,.22,-.65)),rim,base,rough,metal);
 if(chapter==4){
  vec3 leftLamp=vec3(-19,5,-9)-world,rightLamp=vec3(19,5,-9)-world;
  result+=light(N,V,normalize(leftLamp),vec3(6.8,2.8,.70)/(1+dot(leftLamp,leftLamp)*.025),base,rough,metal);
  result+=light(N,V,normalize(rightLamp),vec3(6.8,2.8,.70)/(1+dot(rightLamp,rightLamp)*.025),base,rough,metal);
 }
 result+=light(N,V,normalize(vec3(-.6,-.32,.7)),vec3(.13,.23,.35),base,rough,metal);
 float sky=N.y*.5+.5;vec3 ambient=mix(vec3(.11,.13,.17),vec3(.27,.35,.44),sky);
 vec3 R=reflect(-V,N);float fres=pow(1-max(dot(N,V),0),3);
 result+=base*ambient*(1-metal*.6)+ambient*(metal*.26+fres*.12);
 result+=vec3(.03,.22,.17)*base/(1+.17*dot(world-hero,world-hero));
 result+=base*emission;
 float fog=1-exp(-length(world-eye)*(chapter==3?.0026:chapter==5?.0018:.0011));
 vec3 fogColor=chapter==3?vec3(.39,.52,.47):chapter==6?vec3(.25,.42,.63):chapter==4?vec3(.024,.028,.028):chapter==1?vec3(.015,.042,.039):vec3(.012,.025,.050);
 result=mix(result,fogColor,fog);
 frag=vec4(result,1);
}
