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
 vec3 result=light(N,V,normalize(vec3(-.50,.82,.65)),vec3(3.0,3.0,3.05),base,rough,metal)*shadow();
 result+=light(N,V,normalize(vec3(.7,.22,-.65)),vec3(.72,.60,1.35),base,rough,metal);
 result+=light(N,V,normalize(vec3(-.6,-.32,.7)),vec3(.13,.23,.35),base,rough,metal);
 float sky=N.y*.5+.5;vec3 ambient=mix(vec3(.11,.13,.17),vec3(.27,.35,.44),sky);
 vec3 R=reflect(-V,N);float fres=pow(1-max(dot(N,V),0),3);
 result+=base*ambient*(1-metal*.6)+ambient*(metal*.26+fres*.12);
 result+=vec3(.03,.22,.17)*base/(1+.17*dot(world-hero,world-hero));
 result+=base*emission;
 float fog=1-exp(-length(world-eye)*.0011);result=mix(result,vec3(.012,.019,.04),fog);
 frag=vec4(result,1);
}
