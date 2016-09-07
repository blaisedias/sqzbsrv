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
#ifndef SONGS_DB_H_INCLUDED
#define SONGS_DB_H_INCLUDED

#include "audio_file_tags.h"

namespace songs_db {
    //audio_file_tags::AudioFileRecordStore* new_record_store();
    audio_file_tags::AudioFileRecordStoreCollection* new_record_store_collection();
}

#endif
