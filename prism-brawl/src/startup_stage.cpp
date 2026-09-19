// SPDX-License-Identifier: GPL-3.0-or-later
#include "startup_stage.hpp"
#include "battle.hpp"
#include <cstdlib>
#include <string_view>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sw {
std::filesystem::path startupStageFile(){
 const char* state=std::getenv("XDG_STATE_HOME");
 if(state&&*state&&std::filesystem::path(state).is_absolute())
  return std::filesystem::path(state)/"prism-brawl/last-starting-stage";
 const char* home=std::getenv("HOME");
 if(home&&*home&&std::filesystem::path(home).is_absolute())
  return std::filesystem::path(home)/".local/state/prism-brawl/last-starting-stage";
 return {};
}

int chooseStartupStage(std::uint64_t seed,const std::filesystem::path& file){
 Random random(seed^0x57a27eULL);
 int chosen=int(random.next()%StageCount);
 if(file.empty())return chosen;
 std::error_code error;
 std::filesystem::create_directories(file.parent_path(),error);
 if(error)return chosen;
 int fd=open(file.c_str(),O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK,0600);
 if(fd<0)return chosen;
 struct Close {int fd;~Close(){close(fd);}} closeFile{fd};
 struct stat info{};
 if(fstat(fd,&info)!=0||!S_ISREG(info.st_mode)||info.st_uid!=geteuid())return chosen;
 if(flock(fd,LOCK_EX)!=0)return chosen;
 char buffer[64]{};ssize_t size=pread(fd,buffer,sizeof(buffer),0);
 if(size>0){
  std::string_view previous(buffer,size);
  if(previous.ends_with('\n'))previous.remove_suffix(1);
  for(int stage=0;stage<StageCount;++stage)if(previous==Stages[stage].key){
   // Draw uniformly from the other arenas, even when the random seed repeats.
   chosen=int(random.next()%(StageCount-1));if(chosen>=stage)++chosen;break;
  }
 }
 std::string saved=std::string(Stages[chosen].key)+'\n';
 if(pwrite(fd,saved.data(),saved.size(),0)==ssize_t(saved.size())){
  if(ftruncate(fd,off_t(saved.size()))==0)fsync(fd);
 }
 return chosen;
}
}
