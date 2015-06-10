#include <iomanip>
#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>

#include "audio_file_tags.h"
#include "audio_tags.h"

using namespace std;

//g++ audio_file_tags.cpp -I /usr/include/taglib -L/usr/lib/x86_64-linux-gnu -ltag
//g++ -c audio_file_tags.cpp -I /usr/include/taglib 
namespace audio_file_tags {

int handle_file(const char * filename, AudioFileRecordStore& record_store)
{
//    if (record_store.record_update_required(filename) == false)
//        return 1;

    TagLib::FileRef f(filename);
    if (!f.isNull() && f.tag() && record_store.record_update_required(filename))
    {
        AudioFileRecord &record = record_store.get_record(filename);
        record.update_start();
if (false)
{
        TagLib::Tag *tag = f.tag();
        std::cout << filename << endl;
        std::cout << "-- TAG (basic) --" << endl;
        std::cout << "title   - \"" << tag->title()   << "\"" << endl;
        std::cout << "artist  - \"" << tag->artist()  << "\"" << endl;
        std::cout << "album   - \"" << tag->album()   << "\"" << endl;
        std::cout << "year    - \"" << tag->year()    << "\"" << endl;
        std::cout << "comment - \"" << tag->comment() << "\"" << endl;
        std::cout << "track   - \"" << tag->track()   << "\"" << endl;
        std::cout << "genre   - \"" << tag->genre()   << "\"" << endl;
}
        TagLib::PropertyMap tags = f.file()->properties();

        for(TagLib::PropertyMap::ConstIterator i = tags.begin(); i != tags.end(); ++i)
        {
//            std::string skey(i->first.toCString(true));
            for(TagLib::StringList::ConstIterator j = i->second.begin(); j != i->second.end(); ++j) 
            {
//                record.update(skey, j->toCString(true));
                record.update(i->first.toCString(true), j->toCString(true));
            }
        }
        if (f.audioProperties())
        {
            TagLib::AudioProperties *properties = f.audioProperties();
            record.update(audio_tags::BITRATE, properties->bitrate());
            record.update(audio_tags::LENGTH, properties->length());
            record.update(audio_tags::SAMPLERATE, properties->sampleRate());
            record.update(audio_tags::CHANNELS, properties->channels());
        }
        record.update_complete();
        return 1;
    }
    return 0;
}

int handle_file(const std::string& filename, AudioFileRecordStore& record_store)
{
    return handle_file(filename.c_str(), record_store);
}

int handle_directory(const char * dirname, AudioFileRecordStore& record_store)
{
    return 1;
}

int handle_directory(const std::string& directory, AudioFileRecordStore& record_store)
{
    return handle_directory(directory.c_str(), record_store);
}

} //namespace
