#version 330 core
in vec3 world,normal,local;in vec2 uv;in vec4 color,mat,shadowCoord;
in vec3 dimensions,localNormal;
uniform sampler2D noiseTex,shadowTex,hullTex,materialAtlas,facadeAtlas;uniform vec3 eye,hero;uniform float time;uniform int chapter;
out vec4 frag;
const float PI=3.14159265;
float n3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3-2*f);vec2 uv=i.xy+vec2(37,17)*i.z+f.xy;return mix(texture(noiseTex,(uv+.5)/256.0).r,texture(noiseTex,(uv+vec2(37,17)+.5)/256.0).r,f.z);}
float fbm(vec3 p){return n3(p)*.54+n3(p*2.03)*.27+n3(p*4.07)*.135+n3(p*8.13)*.065;}
vec3 materialTile(vec2 p,vec2 tile){return pow(texture(materialAtlas,(tile+vec2(.015)+fract(p)*.97)*.5).rgb,vec3(2.2));}
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
  if(chapter==1||chapter==2){
   float grain=fbm(world*.27);
   base=chapter==1?mix(vec3(.032,.065,.052),vec3(.12,.17,.12),grain):mix(vec3(.035,.12,.17),vec3(.24,.40,.44),grain);
   base*=1.-seam*.34;emission=edge*.85;rough=chapter==1?.7:.28;metal=.12;
  }
  if(chapter>=3){
   vec3 asphalt=materialTile(vec2(uv.x*.09,uv.y*2.5),vec2(1,1));
   base=asphalt*(chapter==3?vec3(1.4,1.38,1.2):vec3(.65,.75,.85));
   float dashed=step(.48,fract(uv.x/9.0))*(1-smoothstep(.008,.015,abs(uv.y-.5)));
   base=mix(base,chapter==3?vec3(.72,.67,.36):vec3(.82,.45,.08),dashed);
   base=mix(base,chapter==3?vec3(.38,.57,.25):vec3(.06,.61,.66),edge);
   emission=edge*(chapter==3?.15:1.25);rough=chapter==3?.76:.25;metal=.15;
  }
 } else if(mode>13.5&&mode<14.5){
  // World-size texels and explicit wall axes: side walls must not rotate
  // their floors ninety degrees, or stretch a few windows over 150 metres.
  float style=floor((mode-14.)*10.+.5);
  bool side=abs(localNormal.x)>.5;
  vec2 wall=vec2((side?local.z:local.x)+1.,1.-local.y)*vec2(side?dimensions.z:dimensions.x,dimensions.y)/16.;
  vec2 tile=vec2(mod(style,2.),floor(style/2.));
  vec3 facade=pow(texture(facadeAtlas,(tile+.012+fract(wall)*.976)*.5).rgb,vec3(2.2));
  base=facade*mix(vec3(1.05),color.rgb,.18);
  float warm=smoothstep(.025,.10,facade.r-facade.b)*smoothstep(.08,.35,facade.r);
  emission=.06+warm*(chapter==3?.14:.85);rough=style==2.?.23:.66;metal=style==2.?.55:.12;
  if(abs(localNormal.y)>.5){base=color.rgb*.45;emission=0;rough=.8;metal=.15;}
 } else if(mode>14.5&&mode<15.5){
  float tread=step(.24,fract(uv.x*26.+abs(uv.y-.5)*3.5));
  base*=mix(.55+.45*tread,.80,mat.w);rough=.85;metal=.02;
 }
 if(mode>19.5&&mode<20.5){
  vec3 worn=materialTile(uv*1.1,vec2(0,0));
  vec3 rust=materialTile(uv*1.1,vec2(1,0));
  float wear=clamp(mat.w,0,1);
  float chips=1-smoothstep(.09,.27,dot(worn,vec3(.333)));
  base*=mix(1.0,.55+dot(worn,vec3(.65)),wear*.65);
  base=mix(base,vec3(.27,.30,.31),chips*wear*.8);
  base=mix(base,rust*1.15,wear*wear*.88);rough=clamp(rough+chips*.18,.2,.9);
 }else if(mode>20.5&&mode<21.5){
  vec3 moss=materialTile(world.xz*.065,vec2(0,1));
  float patches=fbm(world*.013),slope=1.-abs(N.y);
  base=mix(vec3(.055,.16,.018),vec3(.23,.36,.055),patches)*(.72+moss*.85);
  base=mix(base,vec3(.11,.13,.09),smoothstep(.25,.7,slope));rough=.96;metal=0;
 }else if(mode>21.5&&mode<22.5){
  vec2 cell=abs(mod(world.xz+26.,52.)-26.);
  float street=1.-smoothstep(7.,8.,min(cell.x,cell.y));
  vec3 asphalt=materialTile(world.xz*.045,vec2(1,1));
  base=mix(vec3(.035,.046,.053),asphalt*.32,street);
  float edge=1.-smoothstep(.10,.22,abs(min(cell.x,cell.y)-7.2));base+=edge*vec3(.08,.10,.10);
  float lane=(1.-smoothstep(.07,.16,min(cell.x,cell.y)))*step(.45,fract(max(world.x,world.z)/7.));
  base+=lane*vec3(.22,.16,.055);rough=mix(.72,.32,street);metal=.08;
 }else if(mode>25.5&&mode<26.5){
  float snow=smoothstep(.40,.86,N.y+.18*fbm(world*.09));
  base=mix(vec3(.045,.12,.19),vec3(.49,.64,.69),snow);rough=mix(.32,.92,snow);metal=.03;
 }else if(mode>23.5&&mode<24.5){
  float angle=atan(local.z,local.x),r=length(local.xz);
  float spokes=pow(.5+.5*cos(angle*5.0+.22*r),18.0);
  // Average the repeated pattern at racing speed, preserving correct distance
  // rotation below the temporal alias threshold without counter-rotation tricks.
  float visibility=exp(-pow(mat.w/7.0,4.0));
  base*=mix(.50,.10+.9*spokes,visibility);rough=.3;metal=.82;
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
 vec3 result=light(N,V,normalize(vec3(-.50,.82,.65)),chapter==4?vec3(1.8,1.85,2.4):vec3(3.0,3.0,3.05),base,rough,metal)*shadow();
 result+=light(N,V,normalize(vec3(.7,.22,-.65)),vec3(.72,.60,1.35),base,rough,metal);
 result+=light(N,V,normalize(vec3(-.6,-.32,.7)),vec3(.13,.23,.35),base,rough,metal);
 float sky=N.y*.5+.5;vec3 ambient=mix(vec3(.11,.13,.17),vec3(.27,.35,.44),sky);
 if(chapter==3)ambient=mix(vec3(.18,.22,.13),vec3(.45,.53,.58),sky);
 vec3 R=reflect(-V,N);float fres=pow(1-max(dot(N,V),0),3);
 result+=base*ambient*(1-metal*.6)+ambient*(metal*.26+fres*.12);
 result+=vec3(.03,.22,.17)*base/(1+.17*dot(world-hero,world-hero));
 result+=base*emission;
 float fog=1-exp(-length(world-eye)*(chapter==3?.00065:chapter==4?.0024:.0015));result=mix(result,chapter==3?vec3(.22,.35,.34):chapter==4?vec3(.031,.045,.076):vec3(.012,.022,.042),fog);
 float alpha=mode>24.5&&mode<25.5?mat.w*pow(max(dot(N,V),0),1.4):1.0;
 frag=vec4(result,alpha);
}
