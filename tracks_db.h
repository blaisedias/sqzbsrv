#ifndef TRACKS_DB_H_INCLUDED
#define TRACKS_DB_H_INCLUDED

namespace tracks_db {
    audio_file_tags::AudioFileRecordStore* new_record_store();
    void initialise();
}
#endif
