#ifndef SONGS_DB_H_INCLUDED
#define SONGS_DB_H_INCLUDED

#include "audio_file_tags.h"

namespace songs_db {
    audio_file_tags::AudioFileRecordStore* new_record_store();
}

#endif
