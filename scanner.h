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
