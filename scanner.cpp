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
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "scanner.h"
#include <iostream>

namespace Scanner {
Scanner::Scanner(audio_file_tags::AudioFileRecordStore& store):record_store(store)
{
}

int Scanner::handle_file(const char *filename)
{
    audio_file_tags::handle_file(filename, record_store);
    return 1;
}

int Scanner::handle_directory(const char *dirname)
{
    return audio_file_tags::handle_directory(dirname, record_store);
}

void Scanner::scan(const char *rootdir)
{
    dirwalk(rootdir);
}
}
