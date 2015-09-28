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
#include "scanner.h"
#include "tracks_db.h"
#include "audio_tags.h"
#include "audio_file_tags.h"
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

void process_command(const char *cmd, audio_file_tags::AudioFileRecordStore& record_store, std::vector<const char *> args)
{
    std::cout << "Command: " << cmd;
    for(unsigned ix=0; ix < args.size(); ix++)
        std::cout << " " << args[ix];
    std::cout << std::endl;

    if (0 == strcmp(cmd, "scan"))
    {
        Scanner::Scanner scanner(record_store);
        for(unsigned ix=0; ix < args.size(); ix++)
            scanner.scan(args[ix]);
    }
    if (0 == strcmp(cmd, "load"))
    {
        audio_tags::load();
        record_store.load();
    }
    if (0 == strcmp(cmd, "save"))
    {
        record_store.save();
        audio_tags::save();
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
  audio_file_tags::AudioFileRecordStore* record_store = tracks_db::new_record_store();
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
  delete record_store;
}


