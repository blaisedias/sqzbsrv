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
#ifndef AUDIO_FILE_TAGS_H_INCLUDED
#define AUDIO_FILE_TAGS_H_INCLUDED
#include <iostream>

namespace audio_file_tags
{
    class AudioFileRecord
    {
        public:
            virtual ~AudioFileRecord(){};
            virtual void update(const char * tag, const char *value)=0;
            virtual void update(const std::string tag, const std::string value)=0;
            virtual void update(const char *, int)=0;
            virtual void update(const std::string tag, int)=0;
            virtual void update_start()=0;
            virtual void update_complete()=0;
            virtual bool update_required()=0;
            virtual void dump()=0;
    };

    class AudioFileRecordStore
    {
        public:
            virtual ~AudioFileRecordStore(){};
            virtual const AudioFileRecord* const find_record(const char *filename)=0;
            virtual AudioFileRecord& get_record(const char *filename)=0;
            virtual AudioFileRecord& get_record(const std::string &)=0;

            virtual bool record_update_required(const char *filename)=0;
            virtual bool record_update_required(const std::string &)=0;

            virtual void remove_record(const std::string &filename)=0;
            virtual void remove_record(const char *filename)=0;

            virtual void refresh_records()=0;
            virtual void save()=0;
            virtual void load()=0;

            virtual void test()=0;
            virtual void dump_records()=0;
    };

    int handle_file(const char *, AudioFileRecordStore& );
    int handle_directory(const char * , AudioFileRecordStore&);
    int handle_file(const std::string&, AudioFileRecordStore& );
    int handle_directory(const std::string&, AudioFileRecordStore&);
    extern bool verbose;
}
#endif
