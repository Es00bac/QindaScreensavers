// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene.hpp"
#include "starward/metrics.hpp"
#include <iostream>
#include <stdexcept>
using namespace sw;
int assertions=0;
void check(bool b,const char* s){assertions++;if(!b)throw std::runtime_error(s);}
int main(){try{
 Frame f;initialize(f);
 for(auto& b:f.batches){check(!b.mesh.v.empty(),"empty mesh");check(b.mesh.ix.size()%3==0,"triangle count");for(auto v:b.mesh.v){check(std::isfinite(v.p.x)&&std::isfinite(v.p.y)&&std::isfinite(v.p.z),"finite positions");check(std::isfinite(v.n.x)&&std::abs(length(v.n)-1)<.01,"unit normals");}for(auto i:b.mesh.ix)check(i<b.mesh.v.size(),"index bounds");}
 Director d;size_t peak=0;
 for(int seed=0;seed<4;seed++){d.seed=seed;for(int j=0;j<840;j++){compose(f,j*.331, d);check(f.chapter>=0&&f.chapter<7,"chapter bounds");check(f.fade>=0&&f.fade<=1,"fade bounds");size_t n=0;for(auto& b:f.batches){n+=b.instances.size();for(auto& i:b.instances)for(float a:i.model.a)check(std::isfinite(a),"finite model");}peak=std::max(peak,n);check(n<6000,"instance bound");}}
 d.fixedChapter=1;compose(f,11,d);V3 old=f.pengu;M4 mount=f.pship;compose(f,11+1./120,d);check(length(f.pengu-old)<.03,"continuous hero position");check(length(point(f.pship,{2.39,-.43,.73})-point(mount,{2.39,-.43,.73}))<.03,"attached muzzle continuity");
 auto c=starward::parseCpu("cpu 10 20 30 40 5 2 3 1 500 900");check(c&&c->total==111&&c->idle==45,"CPU guest not duplicated");auto m=starward::parseMemory("MemTotal: 8388608 kB\nMemAvailable: 2097152 kB\n");check(m&&m->first==6&&m->second==8,"memory available");check(!starward::cpuUsage({10,5},{10,5}),"zero CPU interval");
 std::cout<<assertions<<" assertions passed; peak instances "<<peak<<'\n';return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
