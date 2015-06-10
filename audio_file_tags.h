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
            virtual AudioFileRecord& get_record(const char *filename)=0;
            virtual AudioFileRecord& get_record(const std::string &)=0;

            virtual bool record_update_required(const char *filename)=0;
            virtual bool record_update_required(const std::string &)=0;

            virtual void remove_record(const std::string &filename)=0;
            virtual void remove_record(const char *filename)=0;

            virtual void refresh_records(int (*)(const char *filename, AudioFileRecordStore&))=0;
            virtual void save()=0;
            virtual void load()=0;

            virtual void test()=0;
            virtual void dump_records()=0;
    };

    int handle_file(const char *, AudioFileRecordStore& );
    int handle_directory(const char * , AudioFileRecordStore&);
    int handle_file(const std::string&, AudioFileRecordStore& );
    int handle_directory(const std::string&, AudioFileRecordStore&);
}
#endif
