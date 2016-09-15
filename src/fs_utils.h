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
#ifndef FS_UTILS_H_INCLUDED
#define FS_UTILS_H_INCLUDED
#include <stdint.h>
#include <time.h>

namespace fs_utils
{
    class handler {
        public:
            virtual int handle_file(const char *)=0;
            virtual int handle_directory(const char *)=0;
            virtual ~handler(){};
            void dirwalk(const char * path, bool ignore_symlinks=true );
    };

    extern bool verbose;

    uintmax_t file_size(const std::string& filename);
    uintmax_t file_size(const char* f);
    time_t file_timestamp(const std::string& filename);
    time_t file_timestamp(const char* f);

    void path_append(std::string& path, const char *frag);
    void path_append(std::string& path, const std::string& frag);

    std::string dirname(std::string& filepath);
    std::string dirname(const char* filepath);
    std::string filename(std::string& filepath);
    std::string filename(const char* filepath);
    bool exists(const char* filepath);
}
#endif
