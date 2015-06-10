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
