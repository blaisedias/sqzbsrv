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
#ifndef AUDIO_TAGS_H_INCLUDED
#define AUDIO_TAGS_H_INCLUDED
#include <iostream>
#include <vector>

namespace audio_tags
{
    extern const char * FILEPATH;
    extern const char * DIRECTORY;
    extern const char * FILELENGTH;
    extern const char * FILETIMESTAMP;

    extern const char * BITRATE;
    extern const char * LENGTH;
    extern const char * SAMPLERATE;
    extern const char * CHANNELS;
    extern const char * ALBUM;
    extern const char * ARTIST;
    extern const char * TITLE;
    extern const char * DATE;
    extern const char * TRACKNUMBER;
    extern const char * DISCNUMBER;
    extern const char * ALBUMARTIST;
    extern const char * GENRE;
//    extern const char * FILETYPE;
    extern const char * COMPOSER;
    extern const char * CONDUCTOR;

    void save();
    void load();
    void dump();

    bool is_supported(const std::string &);
    bool is_unsupported(const std::string &);
    int  index_of_supported(const std::string &);
    const std::string& value_of_supported(int ix);
    int extend_supported(const std::string &);

    void get_supported_taglist(std::vector<std::string> &);
    void get_unsupported_taglist(std::vector<std::string> &);
}
#endif
