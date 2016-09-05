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
#ifndef SCANNER_H_INCLUDED
#define SCANNER_H_INCLUDED
#include "fs_utils.h"
#include "audio_file_tags.h"

namespace Scanner {

class Scanner : public fs_utils::handler {
    private:
        audio_file_tags::AudioFileRecordStore& record_store; //share_ptr?
    public:
        Scanner(audio_file_tags::AudioFileRecordStore&);
        int handle_file(const char *filename);
        int handle_directory(const char *dirname);
        void scan(const char *rootdir);
};

}
#endif
