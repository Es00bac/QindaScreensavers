#version 330 core
in vec3 world,normal,local;in vec2 uv;in vec4 color,mat,shadowCoord;
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
 if(mat.z>11.5){
  float r=length(local.xz),a=atan(local.z,local.x);float warp=sin(a*4.0-r*12.0+time*.27);
  float strand=pow(.5+.5*sin(a*7.0+pow(r,.6)*24.0-time*.55+warp),15.0);
  float edge=pow(clamp(r,0,1),14);vec3 pool=vec3(.002,.006,.021)+vec3(.035,.17,.32)*strand*r*(1-r)*4;
  pool+=vec3(.015,.25,.45)*edge*.55;pool+=vec3(.05,.024,.1)*pow(1-r,3);
  frag=vec4(pool*color.a,1);return;
 }
 vec3 N=normalize(normal),V=normalize(eye-world),base=color.rgb;
 float rough=mat.x,metal=mat.y,mode=mat.z;float emission=color.a;
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
 vec3 result=light(N,V,normalize(vec3(-.50,.82,.65)),vec3(3.2,2.9,2.55),base,rough,metal)*shadow();
 result+=light(N,V,normalize(vec3(.7,.22,-.65)),vec3(.38,.92,1.25),base,rough,metal);
 result+=light(N,V,normalize(vec3(-.6,-.32,.7)),vec3(.13,.23,.35),base,rough,metal);
 float sky=N.y*.5+.5;vec3 ambient=mix(vec3(.11,.13,.17),vec3(.27,.35,.44),sky);
 vec3 R=reflect(-V,N);float fres=pow(1-max(dot(N,V),0),3);
 result+=base*ambient*(1-metal*.6)+ambient*(metal*.26+fres*.12);
 result+=vec3(.03,.22,.17)*base/(1+.17*dot(world-hero,world-hero));
 result+=base*emission;
 float fog=1-exp(-length(world-eye)*.0022);result=mix(result,vec3(.012,.019,.04),fog);
 frag=vec4(result,1);
}
