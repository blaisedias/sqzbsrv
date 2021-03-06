/*

Copyright (C) 2014,2015  Blaise Dias

This file is part of sqzbsrv.

sqzbsrv is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

sqzbsrv is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sqzbsrv.  If not, see <http://www.gnu.org/licenses/>.
*/
//
// g++ -o rtags rtags.cpp fs_utils.o audio_file_tags.o -lboost_filesystem -lboost_system -L/usr/lib/x86_64-linux-gnu -ltag
//
#include <iostream>
#include <string.h>
#include "scanner.h"
#include "audio_tags.h"
#include "audio_file_tags.h"
#include "tracks_db.h"
#include "songs_db.h"
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

void process_command(const char *cmd, audio_file_tags::AudioFileRecordStoreCollection& record_store_collection, std::vector<const char *> args)
{
    std::cout << "Command: " << cmd;
    for(unsigned ix=0; ix < args.size(); ix++)
        std::cout << " " << args[ix];
    std::cout << std::endl;

    if (0 == strcmp(cmd, "scan"))
    {
        profile_start("scan.prof");
        for(unsigned ix=0; ix < args.size(); ix++)
            record_store_collection.scan(args[ix]);
        profile_stop();
    }
    if (0 == strcmp(cmd, "load"))
    {
        profile_start("load.prof");
        audio_tags::load();
        std::cerr << " audio_tags loaded" << std::endl;
        for(unsigned ix=0; ix < args.size(); ix++)
            record_store_collection.load(args[ix]);
        std::cerr << " record_store loaded" << std::endl;
        profile_stop();
    }
    if (0 == strcmp(cmd, "save"))
    {
        profile_start("save.prof");
        record_store_collection.save();
        audio_tags::save();
        profile_stop();
    }
    if (0 == strcmp(cmd, "dump"))
    {
        record_store_collection.dump_records();
    }
    if (0 == strcmp(cmd, "update"))
    {
        record_store_collection.update_records();
    }
    if (0 == strcmp(cmd, "refresh"))
    {
        record_store_collection.refresh_records();
    }
    if (0 == strcmp(cmd, "test"))
    {
        record_store_collection.test();
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
  audio_file_tags::AudioFileRecordStoreCollection* record_store_collection = 0;
//  audio_file_tags::AudioFileRecordStore* record_store = tracks_db::new_record_store();
//  audio_file_tags::AudioFileRecordStore* record_store = songs_db::new_record_store();
  if (argc > 1)
  {
      int ix_argv = 1;
      while (ix_argv < argc)
      {
          const char *cmd = argv[ix_argv];
          ix_argv++;
          args.clear();
          if (0 == strcmp(cmd, "type"))
          {
              if (ix_argv < argc)
              {
                  if (0 == strcmp(argv[ix_argv], "tracks"))
                      record_store_collection = tracks_db::new_record_store_collection();
                  if (0 == strcmp(argv[ix_argv], "songs"))
                      record_store_collection = songs_db::new_record_store_collection();
              }
              ix_argv++;
              continue;
          }
          if (0 == strcmp(cmd, "verbose"))
          {
              fs_utils::verbose=true;
              audio_file_tags::verbose=true;
              std::cerr << fs_utils::verbose << std::endl;
              std::cerr << audio_file_tags::verbose << std::endl;
              continue;
          }
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

          process_command(cmd, *record_store_collection, args);
      }
  }
  std::cout << "Finished commands. " << std::endl;
  std::cout << "delete_record_store_collection. " << std::endl;
  delete record_store_collection;
  std::cout << "Done. " << std::endl;
}


