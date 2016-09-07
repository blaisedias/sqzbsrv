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
#include <iostream>
#include <string.h>
#include "audio_tags.h"
#include "audio_file_tags.h"
#include "tracks_db.h"
#include <vector>

const char *commands [] = {
    "nop",
    "load",
    "save",
    "scan",
    "update",
    "refresh",
    "dump",
    "redact",
    "test",
};

void process_command(const char *cmd, audio_file_tags::AudioFileRecordStoreCollection& record_store_collection, std::vector<const char *> args)
{
    std::cout << "Command: " << cmd;
    for(unsigned ix=0; ix < args.size(); ix++)
        std::cout << " " << args[ix];
    std::cout << std::endl;

    if (0 == strcmp(cmd, "scan"))
    {
        for(unsigned ix=0; ix < args.size(); ix++)
            record_store_collection.scan(args[ix]);
    }
    if (0 == strcmp(cmd, "load"))
    {
        audio_tags::load();
        for(unsigned ix=0; ix < args.size(); ix++)
            record_store_collection.load(args[ix]);
    }
    if (0 == strcmp(cmd, "save"))
    {
        record_store_collection.save();
        audio_tags::save();
    }
    if (0 == strcmp(cmd, "dump"))
    {
        record_store_collection.dump_records();
    }
    if (0 == strcmp(cmd, "update"))
    {
    }
    if (0 == strcmp(cmd, "refresh"))
    {
        record_store_collection.refresh_records();
    }
    if (0 == strcmp(cmd, "test"))
    {
        record_store_collection.test();
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
  audio_file_tags::AudioFileRecordStoreCollection* record_store_collection = tracks_db::new_record_store_collection();
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

          process_command(cmd, *record_store_collection, args);
      }
  }
  delete record_store_collection;
}


