// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"

namespace sw {
namespace {
float damp(float from,float to,float rate,float dt){return from+(to-from)*(1-std::exp(-rate*dt));}
float envelope(float t,float in,float out,float end){return ease(0,in,t)*(1-ease(out,end,t));}

RigPose gesturePose(int id,int gesture,float t,bool victory){
 RigPose p;
 float beat=std::sin(t*9),sway=std::sin(t*4);
 p.joy=.55f;p.smug=.5f;p.eyes=.82f;p.head.z=.12f*sway;
 p.body.y=.035f*std::sin(t*8);p.torso.y=.12f*sway;
 // Two shared gestures supplement eight distinct signature performances.
 if(gesture==1){ // A planted, forward-leaning "come on" beckon.
  p.body={0,-.08f,.06f};p.torso={.12f,-.25f,-.12f};p.head={-.08f,.20f,.18f};
  p.hands[0]={-.42f,.13f,.05f};p.elbows[0]={-.72f,.24f,.05f};
  p.hands[1]={.44f,.70f,1.02f+.16f*beat};p.elbows[1]={.65f,.39f,.40f};
  p.wrist[1]=-.55f+.28f*beat;p.open[1]=.6f+.35f*beat;
  p.feet[0].z=-.13f;p.feet[1].z=.30f;p.knees[1].z=.27f;p.smug=1;
 }else if(gesture==2){ // Slow, exaggerated applause, shoulders and knees included.
  float clap=.5f+.5f*std::sin(t*11);
  p.body.y=-.06f+.04f*beat;p.torso.x=-.09f;p.head={-.18f,.1f,.12f};
  for(int i=0;i<2;++i){float side=i?1.f:-1.f;
   p.hands[i]={side*(.15f+.36f*clap),.72f+.05f*beat,.73f};
   p.elbows[i]={side*.72f,.42f,.30f};p.open[i]=1;p.wrist[i]=-side*.6f;
   p.knees[i].z+=.08f;
  }
  p.joy=1;p.mouth=.45f;p.eyes=.55f;
 }else switch(id){
 case 0: // Hammer salute, a proud chest-out lean, then a nod.
  p.torso={-.18f,.22f,-.08f};p.head={.13f*beat,-.18f,.09f};
  p.hands[1]={.55f,1.24f,.17f};p.elbows[1]={.76f,.65f,.10f};p.wrist[1]=-.32f+.12f*sway;
  p.hands[0]={-.44f,.10f,.08f};p.elbows[0]={-.73f,.27f,.0f};p.feet[0].x=-.39f;p.feet[1].x=.39f;
  p.smug=.8f;break;
 case 1: // Tip the sunglasses, then offer a swaggering shrug.
  p.torso={-.1f,-.22f,.16f};p.head={-.17f,.24f,-.18f};
  p.hands[1]={.54f,1.21f,.67f};p.elbows[1]={.88f,.62f,.34f};p.open[1]=.4f;
  p.hands[0]={-.83f,.52f,.43f};p.elbows[0]={-.61f,.19f,.20f};p.open[0]=1;p.wrist[0]=-1.0f;
  p.feet[1].z=.28f;p.smug=1;break;
 case 2: // Fox bow, sweeping arm and a tail flick.
  p.torso={.24f+.10f*sway,.25f,-.19f};p.body.y=-.08f;p.head={-.28f,-.23f,.22f};
  p.hands[0]={-.19f,.51f,.61f};p.elbows[0]={-.75f,.33f,.41f};
  p.hands[1]={.98f,.63f+.16f*beat,.15f};p.elbows[1]={.71f,.27f,.06f};p.open[1]=1;
  p.feet[0].z=.35f;p.feet[1].z=-.18f;p.tail=1;p.ears=.3f;p.smug=1;break;
 case 3: // Look at the wrist, then double over with a cheeky laugh.
  p.body.y=-.07f+.04f*beat;p.torso={.12f+.09f*beat,-.15f,.10f};p.head={.06f,-.25f,.14f};
  p.hands[0]={-.23f,.73f,.72f};p.elbows[0]={-.61f,.46f,.42f};p.wrist[0]=-.7f;
  p.hands[1]={.16f,.76f,.76f};p.elbows[1]={.71f,.25f,.30f};p.open[1]=.5f;
  p.eyes=.4f;p.joy=1;p.mouth=.7f;p.tail=.7f;break;
 case 4: // Rabbit bounce, alternating feet and an enormous double wave.
  p.body.y=.12f*std::abs(beat);p.torso.z=.16f*sway;p.head.z=-.18f*sway;
  for(int i=0;i<2;++i){float side=i?1.f:-1.f;
   p.hands[i]={side*(.74f+.12f*sway),1.31f+side*.18f*beat,.24f};
   p.elbows[i]={side*.78f,.72f,.12f};p.open[i]=1;p.wrist[i]=side*.4f*sway;
   p.feet[i].y+=.12f*std::max(0.f,side*beat);p.knees[i].z+=.13f;
  }
  p.ears=.65f*beat;p.joy=1;p.mouth=.32f;p.eyes=.75f;break;
 case 5: // Cat inspects a raised paw and dismissively flicks it away.
  p.torso={-.14f,.28f,-.12f};p.head={.10f,-.30f,-.13f};
  p.hands[1]={.38f+.12f*sway,1.03f,.74f};p.elbows[1]={.73f,.49f,.33f};
  p.wrist[1]=.55f*beat;p.open[1]=.75f;p.hands[0]={-.44f,.12f,.07f};
  p.elbows[0]={-.73f,.28f,.07f};p.feet[1].z=.30f;p.eyes=.55f;p.smug=1;p.tail=1;break;
 case 6: // Panda chest drums, a wide stance and a full-body roar.
  p.body.y=-.10f+.055f*beat;p.torso={-.17f+.09f*beat,0,.08f*sway};p.head.x=-.22f;
  for(int i=0;i<2;++i){float side=i?1.f:-1.f,drum=.5f+.5f*std::sin(t*13+side*pi/2);
   p.hands[i]={side*(.25f+.40f*drum),.60f,.63f+.10f*drum};p.elbows[i]={side*.85f,.26f,.20f};
   p.feet[i].x=side*.43f;p.knees[i]={side*.42f,-.29f,.20f};
  }
  p.mouth=.85f;p.anger=.25f;p.joy=.8f;break;
 case 7: // Axolotl's big side-to-side wave, fins/gills and feet follow.
  p.torso.z=.22f*sway;p.head.z=-.18f*sway;p.body.y=.045f*beat;
  for(int i=0;i<2;++i){float side=i?1.f:-1.f;
   p.hands[i]={side*(.86f+.13f*beat),.96f+side*.33f*sway,.25f};p.elbows[i]={side*.72f,.41f,.17f};
   p.open[i]=1;p.wrist[i]=side*.7f*beat;p.feet[i].z=side*.16f*sway;
  }
  p.ears=.8f;p.joy=1;p.mouth=.40f;break;
 }
 if(victory){p.joy=1;p.smug=.7f;p.body.y+=.07f*std::max(0.f,beat);p.head.x-=.10f;}
 return p;
}

RigPose poseFor(const Fighter& f,double time){
 const auto& a=f.anim;float t=f.actionTime,clock=float(std::fmod(time,4096.0));
 RigPose p;
 float breath=std::sin(clock*(2.4f+f.id*.09f)+f.id*1.7f);
 float speed=clamp(std::abs(f.vx)/7),air=a.air,land=a.land;
 p.body.y=.014f*breath-.21f*land;
 p.torso={.16f*a.walk, .065f*std::sin(a.gait)*a.walk, -.04f*std::sin(a.gait)*a.walk};
 p.head={-.08f*a.walk,a.look*.22f,-.035f*breath};p.head.x-=a.lookUp*.17f;
 p.squash=1-.14f*land;p.anger=.17f;p.eyes=1;
 for(int i=0;i<2;++i){float side=i?1.f:-1.f,phase=a.gait+(i?0:pi);
  float stride=std::cos(phase)*.40f*a.walk,lift=std::max(0.f,std::sin(phase))*.23f*a.walk;
  p.feet[i]={side*(.28f+.025f*speed),-.54f+lift+air*(i?.22f:.10f),.08f+stride+air*(i?-.18f:.23f)};
  p.knees[i]={side*.28f,-.35f+lift*.65f+air*.16f,.07f+stride*.60f+air*.18f+land*.16f};
  p.footPitch[i]=-stride*.75f+air*(i?.42f:-.30f);
  p.hands[i]={side*(.50f+.06f*breath),.36f+.03f*breath-std::cos(phase)*.18f*a.walk,.42f-stride*.70f};
  p.elbows[i]={side*.65f,.21f-std::cos(phase)*.09f*a.walk,.14f-stride*.35f};
 }
 // Acceleration and braking pull the shoulders ahead of or behind the feet.
 p.torso.x+=a.brake*.20f;p.head.x-=a.brake*.13f;
 if(f.action==Action::Jump||f.action==Action::Recovery){
  float rising=clamp(f.vy/13),falling=clamp(-f.vy/18);
  p.torso.x=-.16f*rising+.19f*falling;p.head.x=-.16f*rising;
  p.body.y=.07f*rising;p.squash=1+.07f*rising;
  for(int i=0;i<2;++i){float side=i?1.f:-1.f;
   p.hands[i]={side*(.61f+.28f*falling),.39f+.40f*rising,.20f};
   p.elbows[i]={side*.76f,.33f,.03f};p.open[i]=falling*.65f;
   p.feet[i].y+=.10f*rising;p.knees[i].z+=.15f*rising;
  }
  if(f.action==Action::Recovery){
   p.torso.x=-.28f;p.hands[1]={.36f,1.41f,.41f};p.elbows[1]={.65f,.83f,.24f};
   p.hands[0]={-.66f,.06f,-.05f};p.feet[0].z=-.26f;p.feet[1].z=.20f;p.anger=.75f;
  }
 }
 if(f.action==Action::Jab||f.action==Action::Heavy||f.action==Action::Aerial||f.action==Action::Grab||f.action==Action::Special){
  const auto m=move(f.action,f.id);
  float wind=ease(0,m.start*.62f,t)*(1-ease(m.start*.70f,m.start+.015f,t));
  float snap=ease(m.start-.04f,m.start+.035f,t)*(1-ease(m.end,m.duration,t));
  float follow=ease(m.start+.03f,m.end+.07f,t)*(1-ease(m.duration-.13f,m.duration,t));
  p.anger=.80f;p.eyes=.86f;p.head.x=-.12f*snap;p.body.y-=.10f*wind;
  p.squash-=.07f*wind;p.torso.x=-.16f*wind+.24f*snap;
  if(f.action==Action::Jab){
   int hand=f.variant==1?0:1;float side=hand?1.f:-1.f;
   p.torso.y=side*(-.40f*wind+.38f*snap);p.head.y=-p.torso.y*.5f;
   p.hands[hand]={side*(.46f-.22f*snap),.38f+(f.variant==2?.78f:.11f)*snap,.37f-.44f*wind+1.24f*snap};
   p.elbows[hand]={side*(.67f-.27f*snap),.17f+.30f*snap,.13f+.63f*snap};
   p.hands[1-hand]={-side*.35f,.67f,.49f};p.elbows[1-hand]={-side*.65f,.35f,.12f};
   p.feet[hand].z+=.24f*snap;p.knees[hand].z+=.13f*snap;
  }else if(f.action==Action::Heavy){
   p.torso.y=-.62f*wind+.63f*snap;p.torso.x=-.27f*wind+.39f*snap;
   p.body.z=.18f*snap;p.head.y=-p.torso.y*.60f;
   if(f.id==0||f.variant==0){
    p.hands[1]={.34f,.35f+1.10f*wind+.12f*snap,.30f-.48f*wind+1.15f*snap};
    p.elbows[1]={.66f,.27f+.58f*wind,.17f+.57f*snap};p.wrist[1]=-.5f*wind+.9f*snap;
   }else if(f.variant==1){
    p.hands[1]={.49f+.36f*wind-.51f*snap,.61f+.10f*snap,.22f-.25f*wind+1.24f*snap};
    p.elbows[1]={.80f-.41f*snap,.39f,.10f+.76f*snap};
   }else{
    p.hands[1]={.46f-.16f*snap,.30f-.25f*wind+.95f*snap,.28f+1.02f*snap};
    p.elbows[1]={.69f,.04f+.65f*snap,.10f+.42f*snap};
   }
   p.hands[0]={-.44f,.51f,.28f+.32f*follow};p.elbows[0]={-.75f,.23f,.04f};
   p.feet[0].z=-.24f;p.feet[1].z=.29f;p.knees[1].z=.32f;
  }else if(f.action==Action::Aerial){
   p.torso={-.28f*snap,.38f*wind-.26f*snap,.15f*snap};
   int leg=f.variant==1?0:1,other=1-leg;float side=leg?1.f:-1.f;
   p.feet[leg]={side*.31f,-.40f+.70f*snap,.06f+1.05f*snap};
   p.knees[leg]={side*.28f,-.20f+.37f*snap,.16f+.57f*snap};p.footPitch[leg]=-.9f*snap;
   p.feet[other]={-side*.34f,-.31f,-.12f-.45f*snap};p.knees[other].y=-.13f;
   if(f.variant==2){for(int i=0;i<2;++i){p.feet[i].y+=.32f*snap;p.knees[i].y+=.20f*snap;}}
   for(int i=0;i<2;++i){float s=i?1.f:-1.f;p.hands[i]={s*(.56f+.34f*snap),.43f+.28f*snap,.07f};p.elbows[i]={s*.67f,.22f,.0f};}
  }else if(f.action==Action::Grab){
   p.torso={.23f*snap,-.45f*follow,0};p.head.y=.22f*follow;
   for(int i=0;i<2;++i){float side=i?1.f:-1.f;
    p.hands[i]={side*(.48f-.24f*snap),.45f+.36f*follow,.37f+1.02f*snap-.60f*follow};
    p.elbows[i]={side*.61f,.24f+.24f*follow,.12f+.44f*snap};p.open[i]=1-follow;
   }
   p.feet[0].z=-.20f;p.feet[1].z=.29f;
  }else{ // Individually choreographed casts, timed to the real special release.
   p.torso.y=-.26f*wind+.24f*snap;
   for(int i=0;i<2;++i){float side=i?1.f:-1.f;
    p.hands[i]={side*(.43f+.17f*wind),.39f+.54f*wind,.34f+snap*.73f};
    p.elbows[i]={side*.69f,.24f+.31f*wind,.16f+snap*.35f};p.open[i]=snap;
   }
   switch(f.id){
   case 0:p.hands[1]={.40f,.38f+1.30f*wind-.27f*snap,.31f+.85f*snap};p.elbows[1].y+=.72f*wind;p.torso.x+=.30f*snap;p.wrist[1]=snap;break;
   case 1:p.hands[0].x-=.24f*snap;p.hands[1].x+=.24f*snap;p.torso.x=-.21f*snap;break;
   case 2:p.torso.x=.59f*snap;p.body.y-=.18f*snap;p.hands[0].z=-.43f*snap;p.hands[1].z=1.36f*snap;p.feet[0].z=-.40f*snap;p.tail=1;break;
   case 3:p.torso.y=-.55f*wind+.85f*snap;p.hands[1].x=.57f-.69f*snap;p.hands[1].z=1.40f*snap;p.hands[0].x=-.91f;p.head.y=-p.torso.y*.5f;break;
   case 4:p.hands[1]={.29f,.36f+1.27f*snap,.61f};p.elbows[1].y+=.65f*snap;p.torso.x=-.22f*snap;p.feet[0].y+=.26f*snap;p.ears=snap;break;
   case 5:p.hands[0].x=-.87f;p.hands[1].x=.87f;p.hands[0].y+=.39f*snap;p.hands[1].y+=.39f*snap;p.torso.x=-.18f*snap;p.smug=.65f;break;
   case 6:p.body.y-=.25f*wind;p.squash-=.14f*wind;p.feet[0].x=-.45f;p.feet[1].x=.45f;p.hands[0].y+=.68f*wind;p.hands[1].y+=.68f*wind;p.torso.x=.35f*snap;p.mouth=.6f*snap;break;
   case 7:p.hands[0]={-.79f,.62f+.45f*snap,.60f};p.hands[1]={.79f,.62f+.45f*snap,.60f};p.torso.x=-.15f*snap;p.joy=.8f;p.ears=.8f;break;
   }
  }
 }
 if(f.action==Action::Shield){
  p.body={0,-.13f,-.03f};p.torso={.22f,.13f,-.08f};p.head.x=.06f;p.anger=.7f;
  p.hands[0]={-.29f,.92f,.70f};p.hands[1]={.31f,.73f,.77f};
  p.elbows[0]={-.60f,.48f,.29f};p.elbows[1]={.59f,.36f,.28f};
  p.knees[0].z+=.16f;p.knees[1].z+=.16f;p.feet[0].z=-.20f;p.feet[1].z=.22f;
 }
 if(f.action==Action::Dodge){
  float tuck=envelope(t,.07f,.29f,.42f);
  p.body.y-=.24f*tuck;p.squash-=.14f*tuck;p.torso.x=.42f*tuck;p.head.x=.25f*tuck;
  for(int i=0;i<2;++i){float side=i?1.f:-1.f;
   p.hands[i]={side*.29f,.86f,.37f};p.elbows[i]={side*.52f,.45f,.18f};
   p.feet[i]={side*.22f,-.54f+.42f*tuck,.08f+.26f*tuck};p.knees[i]={side*.27f,-.24f+.27f*tuck,.32f};
  }
  p.eyes=.65f;
 }
 if(f.action==Action::Land){
  float brace=envelope(t,.035f,.075f,.29f);
  p.body.y-=.21f*brace;p.torso.x+=.38f*brace;p.head.x=-.22f*brace;
  p.hands[0]={-.73f,.07f,.59f};p.hands[1]={.66f,.31f,.33f};
  p.elbows[0]={-.64f,.14f,.16f};p.elbows[1]={.71f,.19f,.07f};
  p.feet[0].x=-.42f;p.feet[1].x=.42f;p.knees[0].z+=.23f*brace;p.knees[1].z+=.23f*brace;
  p.pain=f.launchPower*.5f*brace;p.eyes=1-.4f*brace;
 }
 if(f.action==Action::Hurt&&f.floor<0){
  float force=f.launchPower,flail=std::sin(t*18+f.id),flail2=std::sin(t*15+.8f);
  p.body.y=.09f*force;p.torso={-.40f-.22f*force,.23f*flail*force,.19f*flail2*force};
  p.head={-.26f,.20f*flail,-.22f*flail2};p.squash=1+.13f*a.impact;
  for(int i=0;i<2;++i){float side=i?1.f:-1.f;
   p.hands[i]={side*(.75f+.32f*force),.71f+side*.30f*flail,.11f-.40f*force};
   p.elbows[i]={side*.76f,.39f+side*.18f*flail,-.18f};p.open[i]=1;
   p.feet[i]={side*(.32f+.16f*force),-.36f+side*.17f*flail2,.03f+side*.48f*flail};
   p.knees[i]={side*.34f,-.10f+side*.10f*flail2,.24f};p.footPitch[i]=side*.55f*flail;
  }
  p.shock=force;p.pain=1-force*.3f;p.mouth=.35f+.58f*force;p.ears=-.75f;p.tail=-.8f;
 }
 if(f.action==Action::Taunt||f.action==Action::Celebrate){
  bool victory=f.action==Action::Celebrate;
  float weight=victory?ease(0,.28f,t):envelope(t,.22f,1.27f,1.65f);
  p=blendPose(p,gesturePose(f.id,f.gesture,t,victory),weight);
 }
 if(f.action==Action::Defeat){
  p.body.y=-.10f;p.torso={.27f,0,.09f*std::sin(t*2)};p.head={.48f,.10f,.12f};
  p.hands[0]={-.57f,.03f,.20f};p.hands[1]={.57f,.03f,.20f};p.eyes=.5f;p.pain=.5f;p.ears=-.8f;
 }
 // Short reactions layer on top of whatever the fighter is doing, without
 // cancelling attacks or extending hitstun. Only a real hit cancels a taunt.
 float reaction=clamp(f.reactionTime/.18f);
 if(f.reaction==Reaction::Block){
  p.torso.x-=.25f*reaction;p.head.x-=.14f*reaction;p.pain=.30f*reaction;
  for(int i=0;i<2;++i)p.hands[i].z-=.18f*reaction;
 }else if(f.reaction==Reaction::Dizzy){
  p.torso.z=.19f*std::sin(t*8)*reaction;p.head.z=-.34f*std::sin(t*9)*reaction;
  p.head.x=.19f*reaction;p.eyes=.30f;p.shock=.65f*reaction;p.mouth=.40f;p.ears=-1;
  p.hands[0]={-.58f,.05f,.27f};p.hands[1]={.58f,.05f,.27f};
 }else if(f.reaction==Reaction::NearMiss){
  p.head.z+=.23f*reaction;p.shock=.8f*reaction;p.eyes+=.22f*reaction;
 }else if(f.reaction==Reaction::Happy){p.joy=std::max(p.joy,.9f*reaction);p.smug=.6f*reaction;p.ears=.5f*reaction;
 }else if(f.reaction==Reaction::Challenge){p.anger=.9f*reaction;p.head.x-=.14f*reaction;p.ears=-.2f*reaction;}
 return p;
}
}

RigPose blendPose(const RigPose& from,const RigPose& to,float weight){
 RigPose p;weight=clamp(weight);
 p.body=mix(from.body,to.body,weight);p.torso=mix(from.torso,to.torso,weight);p.head=mix(from.head,to.head,weight);
 for(int i=0;i<2;++i){
  p.hands[i]=mix(from.hands[i],to.hands[i],weight);p.elbows[i]=mix(from.elbows[i],to.elbows[i],weight);
  p.feet[i]=mix(from.feet[i],to.feet[i],weight);p.knees[i]=mix(from.knees[i],to.knees[i],weight);
  p.wrist[i]=from.wrist[i]+(to.wrist[i]-from.wrist[i])*weight;
  p.footPitch[i]=from.footPitch[i]+(to.footPitch[i]-from.footPitch[i])*weight;
  p.open[i]=from.open[i]+(to.open[i]-from.open[i])*weight;
 }
#define BLEND(field) p.field=from.field+(to.field-from.field)*weight
 BLEND(squash);BLEND(anger);BLEND(joy);BLEND(shock);BLEND(pain);BLEND(smug);BLEND(eyes);BLEND(mouth);BLEND(ears);BLEND(tail);
#undef BLEND
 return p;
}

void animateFighter(Fighter& f,float dt,double time){
 auto& a=f.anim;
 a.impact=damp(a.impact,0,13,dt);
 a.taunt=damp(a.taunt,f.action==Action::Taunt?1.f:0.f,16,dt);
 a.celebrate=damp(a.celebrate,f.action==Action::Celebrate?1.f:0.f,10,dt);
 if(f.action==Action::Hurt&&f.floor<0&&f.launchPower>.18f){
  a.tumble+=f.spinVelocity*dt;
 }else if(f.action==Action::Dodge){
  a.tumble=f.spinStart-f.face*2*pi*ease(.035f,.39f,f.actionTime);
 }else if(f.action==Action::Aerial&&f.variant==2){
  a.tumble=f.spinStart-f.face*2*pi*ease(.08f,.49f,f.actionTime);
 }else{
  // Settle to the nearest *equivalent* upright angle. Never interpolate across
  // a 2pi wrap: that would produce a one-frame reverse spin at 120 Hz.
  float upright=std::round(a.tumble/(2*pi))*2*pi;
  a.tumble=damp(a.tumble,upright,f.floor>=0?22.f:8.f,dt);
 }
 a.pose=blendPose(a.pose,poseFor(f,time),1-std::exp(-dt*(f.action==Action::Hurt?34.f:24.f)));
}

Fighter animationDemo(int id,double time){
 // A repeatable animation reel for inspecting rigs. The battle renderer never
 // uses it; --showcase --animation-demo opts into these authored situations.
 constexpr double beat=2.4;
 int clip=int(time/beat)%16;float local=float(std::fmod(time,beat));
 Fighter f;f.id=id;f.face=1;f.anim.yaw=.18f;
 for(int tick=0;tick<=int(local/FixedStep);++tick){
  float t=tick*float(FixedStep);f.actionTime=t;f.anim.gait=t*16;
  f.action=Action::Idle;f.floor=0;f.anim.walk=0;f.anim.air=0;
  if(clip==0){f.anim.walk=.9f;f.vx=5;}
  if(clip>=1&&clip<=3){f.action=Action::Taunt;f.gesture=clip-1;}
  if(clip==4||clip==5){f.action=clip==4?Action::Jab:Action::Heavy;f.actionTime=std::fmod(t,1.05f);f.variant=int(t/1.05f)%3;}
  if(clip==6){f.action=Action::Aerial;f.floor=-1;f.anim.air=1;f.variant=1;f.actionTime=std::fmod(t,1.1f);f.y=.35f;}
  if(clip==7){f.action=Action::Special;f.actionTime=std::fmod(t,1.25f);f.anim.cast=envelope(f.actionTime,.22f,.32f,.72f);}
  if(clip==8||clip==9){
   f.launchPower=clip==8?.40f:1;f.spinVelocity=clip==8?-6.f:-13.f;
   if(t<1.05f){f.action=Action::Hurt;f.floor=-1;f.anim.air=1;f.y=.22f+.62f*std::sin(t*pi/1.05f);f.anim.impact=std::max(0.f,1-t*5);}
   else {f.action=Action::Land;f.actionTime=t-1.05f;f.y=0;f.anim.land=std::exp(-(t-1.05f)*12);}
  }
  if(clip==10){f.action=Action::Shield;f.anim.shield=.9f;f.reaction=Reaction::Block;f.reactionTime=std::max(0.f,.45f-std::fmod(t,.8f));}
  if(clip==11){f.action=Action::Hurt;f.reaction=Reaction::Dizzy;f.reactionTime=1;}
  if(clip==12){f.action=t<.42f?Action::Dodge:Action::Idle;}
  if(clip==13){f.action=Action::Recovery;f.floor=-1;f.anim.air=1;f.vy=10;f.y=.25f;}
  if(clip==14){f.action=Action::Celebrate;f.gesture=0;}
  if(clip==15){f.action=Action::Defeat;}
  animateFighter(f,float(FixedStep),time-local+t);
 }
 return f;
}
}
