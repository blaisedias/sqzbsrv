//
// g++ -o rtags rtags.cpp fs_utils.o audio_file_tags.o -lboost_filesystem -lboost_system -L/usr/lib/x86_64-linux-gnu -ltag
//
#include <iostream>
#include <string.h>
#include "sstring.h"
#include "songs_db.h"
#include "scanner.h"
#include "audio_tags.h"
#include "audio_file_tags.h"
#include <vector>

bool profile = false;
#ifdef  PROFILER        
#include <gperftools/profiler.h>
inline void profile_start(const char *pf)
{
    if (profile)
        ProfilerStart(pf);
}
inline void profile_stop()
{
    if (profile)
        ProfilerStop();
}
#else
inline void profile_start(const char *pf)
{
}
inline void profile_stop()
{
}
#endif

const char *commands [] = {
    "nop",
    "load",
    "save",
    "scan",
    "update",
    "refresh",
    "dump",
    "test",
    "profile",
};

void process_command(const char *cmd, audio_file_tags::AudioFileRecordStore& record_store, std::vector<const char *> args)
{
    std::cout << "Command: " << cmd;
    for(unsigned ix=0; ix < args.size(); ix++)
        std::cout << " " << args[ix];
    std::cout << std::endl;

    if (0 == strcmp(cmd, "scan"))
    {
        profile_start("s_rtags.scan.prof");
        Scanner::Scanner scanner(record_store);
        for(unsigned ix=0; ix < args.size(); ix++)
            scanner.scan(args[ix]);
        profile_stop();
    }
    if (0 == strcmp(cmd, "load"))
    {
        profile_start("s_rtags.load.prof");
        sstring::load("data/songs_db_cchars.dat");
        std::cout << " cchars loaded" << std::endl;
        audio_tags::load();
        std::cout << " audio_tags loaded" << std::endl;
        record_store.load();
        std::cout << " record_store loaded" << std::endl;
        profile_stop();
    }
    if (0 == strcmp(cmd, "save"))
    {
        profile_start("s_rtags.save.prof");
        record_store.save();
        audio_tags::save();
        sstring::prune();
        sstring::save("data/songs_db_cchars.dat");
        profile_stop();
    }
    if (0 == strcmp(cmd, "dump"))
    {
        record_store.dump_records();
    }
    if (0 == strcmp(cmd, "update"))
    {
    }
    if (0 == strcmp(cmd, "refresh"))
    {
        record_store.refresh_records(audio_file_tags::handle_file);
    }
    if (0 == strcmp(cmd, "test"))
    {
        record_store.test();
    }
    if (0 == strcmp(cmd, "profile"))
    {
        profile = !profile;
        std::cerr << "Profiling " << profile << std::endl;
    }
}

inline bool is_command(const char *str)
{
    for (unsigned ix=0; ix < sizeof(commands)/sizeof(commands[0]); ix++)
    {
        if (0 == strcmp(str, commands[ix]))
            return true;
    }
    return false;
}

int main( int argc, char* argv[] )
{
  std::vector<const char *> args;
  audio_file_tags::AudioFileRecordStore* record_store = songs_db::new_record_store();
  if (argc > 1)
  {
      int ix_argv = 1;
      while (ix_argv < argc)
      {
          const char *cmd = argv[ix_argv];
          ix_argv++;
          args.clear();
          if (!is_command(cmd))
          {
              std::cerr << "Ignoring " << cmd << std::endl;
              continue;
          }

          // Collect args.
          while((ix_argv < argc) && (!is_command(argv[ix_argv])))
          {
              args.push_back(argv[ix_argv]);
              ix_argv++;
          }

          process_command(cmd, *record_store, args);
      }
  }
  std::cout << "Finished commands. " << std::endl;
  std::cout << "delete_record_store. " << std::endl;
  delete record_store;
  std::cout << "Done. " << std::endl;
}


