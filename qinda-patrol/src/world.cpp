#include "world.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace patrol {
std::uint64_t Rng::next() {
    std::uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}
int Rng::range(int lo, int hi) { return lo + int(next() % std::uint64_t(hi-lo+1)); }
double Rng::unit() { return double(next() >> 11) * (1.0 / 9007199254740992.0); }
const char* biomeName(Biome b) {
    constexpr const char* names[]{"NEON ROOFTOPS", "MEMORY GARDENS", "COOLING DISTRICT", "PACKET DOCKS"};
    return names[int(b)];
}
const char* enemyName(EnemyKind e) {
    constexpr const char* names[]{"BUG", "MEMORY LEAK", "ZOMBIE", "RUNAWAY"};
    return names[int(e)];
}
Biome districtBiome(std::int64_t d) { return Biome(int(((d%4)+4)%4)); }
DistrictMix districtAt(double x) {
    double k=std::floor(x/DistrictLength), local=x-k*DistrictLength;
    auto block=std::int64_t(k);
    auto smooth=[](double u){ u=std::clamp(u,0.0,1.0); return u*u*(3-2*u); };
    if(local<DistrictBlend) return {districtBiome(block-1), districtBiome(block), smooth((local+DistrictBlend)/(2*DistrictBlend))};
    if(local>DistrictLength-DistrictBlend) return {districtBiome(block), districtBiome(block+1), smooth((local-(DistrictLength-DistrictBlend))/(2*DistrictBlend))};
    return {districtBiome(block), districtBiome(block), 0};
}
JumpPlan planJump(Vec from, Vec to) {
    double dx=to.x-from.x, dy=to.y-from.y;
    if(dx<=0 || std::abs(dy)>64 || dx>180) return {};
    // Brisk horizontal pace; climbs and drops buy a little extra hang time.
    double duration=std::clamp(dx/185.0+std::abs(dy)/260.0, 0.58, 1.0);
    double vy=(dy-0.5*Gravity*duration*duration)/duration;
    double t=std::clamp(-vy/Gravity,0.0,duration);
    double apex=from.y+vy*t+0.5*Gravity*t*t;
    return {duration,vy,apex,vy < -180 && vy > -620 && apex < std::min(from.y,to.y)-28};
}
const char* bossName(int k) { static const char* n[]{"SCRAP COLOSSUS", "LEAK HYDRA", "STORM MANTA", "CORE WARDEN"}; return n[k%4]; }
const char* facilityName(Facility k) { static const char* n[]{"RESTORE POWER", "IRRIGATE GARDEN", "PURGE COOLANT", "RELINK ANTENNA"};return n[int(k)]; }
const char* powerName(Power k) { static const char* n[]{"", "RAPID FIRE", "AEGIS SHIELD", "ARC BLASTER"};return n[int(k)]; }
int World::takeBoss() {
    if(bossIndex_==4) { for(int i=3;i>0;--i)std::swap(bossOrder_[i],bossOrder_[bosses_.range(0,i)]); if(bossOrder_[0]==lastBoss)std::swap(bossOrder_[0],bossOrder_[1]);bossIndex_=0; }
    lastBoss=bossOrder_[bossIndex_++];return lastBoss;
}
World::World(std::uint64_t s) { reset(s); }
void World::reset(std::uint64_t s) {
    seed_=s; terrain_={s}; effects_={s^0xda942042e4dd58b5ULL};
    bosses_={s^0x8348a136ULL};bossOrder_={0,1,2,3};bossIndex_=4;lastBoss=-1;nextArena_=7;
    bossesDefeated=repairs=collected=dodges=shieldBlocks=0;victory=0;pickups.clear();hazards.clear();
    nextPlatform_=nextEnemy_=nextSign_=0; sinceSign_=2; time=camera=duckFire=coffeeBeat=0;
    cleared=distanceBase=rescues=0; hero=Hero{}; duck={66,340};
    platforms.clear(); enemies.clear(); bolts.clear(); particles.clear();
    enemies.reserve(32); bolts.reserve(48); particles.reserve(180);
    while(platforms.empty() || platforms.back().x+platforms.back().width<2200) appendPlatform();
    hero.pos.y=platforms.front().y;
}
const Platform* World::platform(std::uint64_t id) const {
    for(const auto& p: platforms) if(p.id==id) return &p;
    return nullptr;
}
Biome World::currentBiome() const { const auto* p=platform(hero.platform); return p?p->biome:Biome::Rooftops; }
void World::appendPlatform() {
    Platform p; p.id=nextPlatform_++;
    if(platforms.empty()) { p.x=0; p.y=416; p.width=448; }
    else {
        const auto& last=platforms.back();
        int gapRoll=terrain_.range(0,99), stepRoll=terrain_.range(0,99);
        int gap=gapRoll<45?1:gapRoll<85?2:3;                                   // 1–3 tiles
        int step=stepRoll<30?0:stepRoll<55?-1:stepRoll<80?1:stepRoll<90?-2:2;  // up to two tiles
        p.x=last.x+last.width+Tile*gap;
        p.y=std::clamp(last.y+Tile*step,320.0,448.0);
        p.width=Tile*terrain_.range(6,18);
    }
    // District lines sit at multiples of DistrictLength in absolute coordinates. A roof
    // never straddles one: it is cut short before the line or, when it starts within six
    // tiles of it, becomes the gate roof that opens the next district.
    double start=p.x+double(distanceBase);
    auto block=std::int64_t(std::floor(start/DistrictLength));
    double line=double(block+1)*DistrictLength;
    if(p.id>0 && start>line-6*Tile) ++block;
    else if(start+p.width>line-Tile) p.width=std::max(5.0*Tile,std::floor((line-Tile-start)/Tile)*Tile);
    p.district=block; p.biome=districtBiome(block);
    p.facility=Facility(int(p.biome));
    // Reserve a complete machinery hall, leaving all generated jumps reachable.
    if(p.id>=nextArena_ && !platforms.empty() && block==platforms.back().district && line-start>26*Tile) {
        p.width=24*Tile;p.arena=true;nextArena_=p.id+7+terrain_.range(0,3);
    }
    p.service=!p.arena && p.id>0 && p.id%3==1;
    p.gate=p.id>0 && block!=platforms.back().district;
    if(p.gate) p.width=std::max(p.width,8.0*Tile);
    p.decoration=terrain_.range(0,5); p.structure=terrain_.range(0,7); p.pattern=terrain_.range(0,3);
    p.cable=terrain_.range(0,2)!=0;
    if(p.id>0 && !p.gate && !p.arena && !p.service && terrain_.range(0,9)<3) p.style=RoofStyle::Catwalk;
    ++sinceSign_;
    if(!p.gate && !p.arena && !p.service && p.width>=10*Tile && sinceSign_>=2) { p.sign=int(nextSign_++%4); sinceSign_=0; }
    p.coffee=(p.id%5==0) && !p.gate && !p.arena && !p.service && p.width>=10*Tile;
    platforms.push_back(p);
    if(p.id==0) return;
    // Enemy count follows roof size; species lean on the district. All fictional.
    int count=p.width<8*Tile?terrain_.range(0,1):p.width>=14*Tile?(terrain_.range(0,9)<4?2:1):(terrain_.range(0,9)<1?0:1);
    bool boss=p.arena; if(boss) count=1;
    static constexpr EnemyKind pools[4][4]{
        {EnemyKind::Zombie,EnemyKind::Drone,EnemyKind::Bug,EnemyKind::Leak},
        {EnemyKind::Leak,EnemyKind::Bug,EnemyKind::Zombie,EnemyKind::Drone},
        {EnemyKind::Bug,EnemyKind::Zombie,EnemyKind::Drone,EnemyKind::Leak},
        {EnemyKind::Drone,EnemyKind::Leak,EnemyKind::Zombie,EnemyKind::Bug}};
    for(int i=0;i<count;++i) {
        Enemy e; e.id=nextEnemy_++; e.platform=p.id;
        e.origin=p.x+p.width*(count==1?0.64:i==0?0.42:0.78);
        e.phase=terrain_.unit()*6.2831853;
        int roll=terrain_.range(0,99); e.kind=pools[int(p.biome)][roll<40?0:roll<70?1:roll<85?2:3];
        e.boss=boss&&i==0; e.hp=e.boss?36:(e.kind==EnemyKind::Zombie?3:2);e.maxHp=e.hp;
        if(e.boss){e.bossKind=takeBoss();e.origin=p.x+p.width*.75;}
        e.pos={e.origin,p.y-(e.kind==EnemyKind::Drone?90:18)};
        enemies.push_back(e);
    }
}
void World::burst(Vec p,int color,int count) {
    for(int i=0;i<count && particles.size()<180;++i) {
        double life=0.35+effects_.unit()*0.5;
        particles.push_back({p,{(effects_.unit()-0.5)*150,-30-effects_.unit()*140},life,life,color});
    }
}
void World::fire(Vec from,const Enemy& enemy,bool byDuck) {
    if(bolts.size()>=48) return;
    double dx=enemy.pos.x-from.x,dy=enemy.pos.y-from.y;
    double length=std::hypot(dx,dy);
    if(length<1) return;
    double speed=byDuck?560:520;
    bolts.push_back({from,{dx/length*speed,dy/length*speed},1.5,byDuck});
    if(byDuck) duckFire=hero.power==Power::Rapid?0.24:0.55; else hero.fire=hero.power==Power::Rapid?0.17:0.4;
}
void World::step(double dt) {
    // Public stepping is bounded too; callers use fixed 1/120 s increments.
    if(!std::isfinite(dt)||dt<=0) return;
    dt=std::min(dt,0.05); time+=dt;
    hero.fire=std::max(0.0,hero.fire-dt); duckFire=std::max(0.0,duckFire-dt);
    hero.flash=std::max(0.0,hero.flash-dt);
    hero.coffeeCooldown-=dt;victory=std::max(0.0,victory-dt);
    hero.powerTime=std::max(0.0,hero.powerTime-dt);if(hero.powerTime==0)hero.power=Power::None;
    hero.dodge=std::max(0.0,hero.dodge-dt);hero.stagger=std::max(0.0,hero.stagger-dt);
    for(auto& item:pickups){item.age+=dt; if(std::hypot(item.pos.x-hero.pos.x,item.pos.y-(hero.pos.y-35))<70){hero.power=item.kind;hero.powerTime=18;item.age=61;++collected;burst(item.pos,1,20);}}
    std::erase_if(pickups,[&](const Pickup& q){return q.age>60||q.pos.x<camera-200;});
    const Platform* p=platform(hero.platform);
    if(!p) { ++rescues; hero.platform=platforms.front().id; hero.pos={platforms.front().x+80,platforms.front().y}; hero.airborne=false; p=&platforms.front(); }
    if(hero.coffee>0) hero.coffee=std::max(0.0,hero.coffee-dt);
    else if(hero.airborne) {
        hero.jumpAge=std::min(hero.jumpAge+dt,hero.jumpDuration);
        double u=hero.jumpAge/hero.jumpDuration;
        hero.pos.x=hero.jumpStart.x+(hero.jumpEnd.x-hero.jumpStart.x)*u;
        hero.pos.y=hero.jumpStart.y+hero.jumpVy*hero.jumpAge+0.5*Gravity*hero.jumpAge*hero.jumpAge;
        if(hero.jumpAge>=hero.jumpDuration) {
            hero.pos=hero.jumpEnd; hero.airborne=false; ++hero.platform;
            burst({hero.pos.x,hero.pos.y-3},0,7);
        }
    } else {
        bool threat=false;
        for(const auto& e:enemies) if(e.hp>0 && e.pos.x>hero.pos.x-24 && e.pos.x<hero.pos.x+230) threat=true;
        double speed=(threat?FightSpeed:RunSpeed)*(hero.stagger>0?.25:1);
        bool arenaLive=false;
        for(const auto& e:enemies)if(e.boss&&e.platform==p->id&&e.hp>0)arenaLive=true;
        if(arenaLive && hero.pos.x>=p->x+p->width*.39) speed=0;
        if(p->service&&!p->repaired&&hero.pos.x>=p->x+p->width*.42){
            speed=0;hero.serviceTime+=dt;
            if(hero.serviceTime>=1.25){for(auto& q:platforms)if(q.id==p->id)q.repaired=true;hero.serviceTime=0;++repairs;
                pickups.push_back({{hero.pos.x+35,p->y-38},Power(1+(repairs-1)%3),0});burst({hero.pos.x+20,p->y-35},0,22);}
        }
        if(p->coffee && hero.coffeeCooldown<=0 && !threat && hero.pos.x>p->x+120 && hero.pos.x<p->x+p->width-90) {
            hero.coffee=2.3; hero.coffeeCooldown=30; coffeeBeat=time;
        } else {
            double edge=p->x+p->width-26;
            hero.pos.x=std::min(edge,hero.pos.x+speed*dt); hero.pos.y=p->y-(hero.dodge>0?std::sin(hero.dodge/.85*3.14159265)*94:0);
            if(hero.pos.x>=edge-0.01) {
                const Platform* next=platform(hero.platform+1);
                if(next) {
                    Vec end{next->x+36,next->y}; auto j=planJump(hero.pos,end);
                    if(j.valid) {
                        hero.airborne=true; hero.jumpAge=0; hero.jumpStart=hero.pos;
                        hero.jumpEnd=end; hero.jumpVy=j.vy; hero.jumpDuration=j.duration;
                        burst({hero.pos.x,hero.pos.y-4},0,6);
                    } else { ++rescues; hero.pos=end; ++hero.platform; }
                }
            }
        }
    }
    Vec wanted{hero.pos.x-65,hero.pos.y-86+std::sin(time*2.7)*8};
    double follow=1.0-std::exp(-6.5*dt);
    duck.x+=(wanted.x-duck.x)*follow; duck.y+=(wanted.y-duck.y)*follow;
    for(auto& e:enemies) {
        const Platform* ep=platform(e.platform); if(!ep) continue;
        e.hit=std::max(0.0,e.hit-dt);
        e.pos.x=e.origin+std::sin(time*1.25+e.phase)*26;
        e.pos.y=ep->y-(e.kind==EnemyKind::Drone?90+std::sin(time*2.6+e.phase)*17:18);
        if(e.boss){
            e.pos.x=e.origin+std::sin(time*.9+e.phase)*18;
            e.pos.y=ep->y-(e.bossKind==2?100+std::sin(time*1.7)*22:55);
            if(hero.platform!=e.platform)continue;
            double cycle=e.hp<e.maxHp/2?3.0:3.8;
            double before=e.attackClock;e.attackClock+=dt;
            double beat=std::fmod(e.attackClock,cycle);e.stage=beat<1?0:beat<1.5?1:2;
            if(std::fmod(before,cycle)<1&&beat>=1){
                hero.dodge=.85;++dodges;
                for(int k=0;k<(e.bossKind==1?5:3);++k){
                    Vec origin=e.pos;Vec velocity{-160.0-k*22,0};int kind=e.bossKind;
                    if(kind==0){origin.y=ep->y-10;velocity.x=-210-k*30;}
                    if(kind==1){origin={hero.pos.x-65+k*55,ep->y-180};velocity={0,130};}
                    if(kind==2){velocity={-240,(k-1)*65.0};}
                    if(kind==3){velocity={-145.0-k*20,-95.0+k*35};}
                    if(hazards.size()<40)hazards.push_back({origin,velocity,2.2,kind==0?12.0:8.0,kind});
                }
            }
        }
    }
    // Local autonomous targeting, not a model or an OS-process controller.
    auto choose=[&](Vec from,bool aerial)->const Enemy* {
        const Enemy* best=nullptr; double cost=1e9;
        for(const auto& e:enemies) if(e.hp>0) {
            if(e.boss&&(hero.platform!=e.platform||e.stage!=2))continue;
            double dx=e.pos.x-from.x,dy=e.pos.y-from.y;
            if(dx<-45 || dx>(e.boss?430:280) || std::abs(dy)>180) continue;
            double c=std::hypot(dx,dy)+(aerial && e.kind!=EnemyKind::Drone?50:0);
            if(c<cost) {cost=c;best=&e;}
        }
        return best;
    };
    if(hero.coffee<=0 && hero.fire<=0 && hero.stagger<=0) if(auto e=choose({hero.pos.x+23,hero.pos.y-35},false)) fire({hero.pos.x+25,hero.pos.y-35},*e,false);
    if(duckFire<=0) if(auto e=choose({duck.x+24,duck.y+5},true)) fire({duck.x+24,duck.y+5},*e,true);
    for(auto& b:bolts) {
        b.life-=dt; b.pos.x+=b.velocity.x*dt; b.pos.y+=b.velocity.y*dt;
        for(auto& e:enemies) if(e.hp>0 && std::hypot(b.pos.x-e.pos.x,b.pos.y-e.pos.y)<(e.boss?31:23)) {
            if(e.boss&&(e.stage!=2||e.platform!=hero.platform)){b.life=0;break;}
            b.life=0;e.hp=std::max(0,e.hp-(hero.power==Power::Arc?3:1));e.hit=0.12;
            burst(b.pos,b.duck?1:0,4);
            if(e.hp==0) { ++cleared; burst(e.pos,int(e.kind)%3,20);
                if(e.boss){++bossesDefeated;victory=3;hazards.clear();pickups.push_back({{hero.pos.x+45,hero.pos.y-38},Power(1+e.bossKind%3),0});for(auto& q:platforms)if(q.id==e.platform)q.repaired=true;}
            }
            break;
        }
    }
    std::erase_if(bolts,[](const Bolt& b){return b.life<=0;});
    for(auto& h:hazards){h.life-=dt;h.pos.x+=h.velocity.x*dt;h.pos.y+=h.velocity.y*dt;if(h.kind==3)h.velocity.y+=180*dt;
        if(p&&(h.kind==1||h.kind==3)&&h.pos.y>=p->y-5){h.life=0;burst({h.pos.x,p->y-5},1,5);}
        double hitDistance=std::hypot(h.pos.x-hero.pos.x,h.pos.y-(hero.pos.y-30));
        if(h.life>0&&hero.power==Power::Shield&&hitDistance<62){h.life=0;++shieldBlocks;burst(h.pos,1,10);}
        else if(h.life>0&&hitDistance<24&&hero.flash<=0){h.life=0;hero.flash=.4;hero.stagger=.28;burst(h.pos,1,6);}}
    std::erase_if(hazards,[&](const Hazard& h){return h.life<=0||h.pos.y>520||h.pos.x<camera-50;});
    for(auto& v:particles) {v.life-=dt;v.pos.x+=v.velocity.x*dt;v.pos.y+=v.velocity.y*dt;v.velocity.y+=220*dt;}
    std::erase_if(particles,[](const Particle& v){return v.life<=0;});
    double desired=std::max(0.0,hero.pos.x-ViewW*0.35);
    if(p&&p->arena&&!p->repaired)desired=std::max(0.0,p->x+p->width*.5-ViewW*.5);
    camera+=(desired-camera)*(1-std::exp(-4*dt));
    maintain();
}
void World::maintain() {
    while(platforms.back().x+platforms.back().width<camera+ViewW+900) appendPlatform();
    while(platforms.size()>3 && platforms.front().x+platforms.front().width<camera-400) platforms.pop_front();
    std::erase_if(enemies,[&](const Enemy& e){return e.hp<=0 || e.pos.x<camera-400;});
    if(camera>65536) {
        constexpr double shift=32768;
        camera-=shift; hero.pos.x-=shift; hero.jumpStart.x-=shift;hero.jumpEnd.x-=shift;duck.x-=shift;distanceBase+=32768;
        for(auto& p:platforms) p.x-=shift;
        for(auto& e:enemies) {e.pos.x-=shift;e.origin-=shift;}
        for(auto& b:bolts) b.pos.x-=shift;
        for(auto& q:particles) q.pos.x-=shift;
        for(auto& q:pickups) q.pos.x-=shift;
        for(auto& q:hazards)q.pos.x-=shift;
    }
}
}
