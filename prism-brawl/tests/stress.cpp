// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
#include <iostream>
using namespace sw;
int main(){std::uint64_t hits=0,kos=0,rounds=0,ticks=0;int cases=0;for(int stage=-1;stage<3;stage++)for(int count:{2,4,8})for(int seed=1;seed<=4;seed++){
 Battle b(seed,stage,count);for(int step=0;step<3600;step++){
  b.advance(.5);auto& s=b.state();if(s.effects.size()>192||s.shots.size()>64)return 1;
  for(int i=0;i<count;i++){auto& f=s.fighters[i];if(!std::isfinite(f.x)||!std::isfinite(f.y)||f.stocks<0||f.damage<0||f.damage>350)return 2;}
 }
 if(b.counters.hits<30||b.counters.rounds<5)return 3;
 hits+=b.counters.hits;kos+=b.counters.kos;rounds+=b.counters.rounds;ticks+=b.counters.ticks;cases++;
 }
 std::cout<<"{\"cases\":"<<cases<<",\"simulated_hours\":"<<cases*.5<<",\"ticks\":"<<ticks<<",\"hits\":"<<hits<<",\"knockouts\":"<<kos<<",\"rounds\":"<<rounds<<"}\n";
}
