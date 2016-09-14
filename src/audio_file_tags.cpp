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
#include <iomanip>
#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>
#include <string.h>

#include "audio_file_tags.h"
#include "audio_tags.h"

using namespace std;

//g++ audio_file_tags.cpp -I /usr/include/taglib -L/usr/lib/x86_64-linux-gnu -ltag
//g++ -c audio_file_tags.cpp -I /usr/include/taglib 
namespace audio_file_tags {

bool verbose = false;

int handle_file(const char* filepath, const char* filekey, AudioFileRecordStore& record_store)
{
    if(record_store.find_record(filekey) == NULL
            || record_store.record_update_required(filekey))
    {
    TagLib::FileRef f(filepath);
    if (!f.isNull() && f.tag())
    {
        AudioFileRecord &record = record_store.get_record(filekey);
        record.update_start();
if (verbose)
{
        TagLib::Tag *tag = f.tag();
        std::cout << filepath << endl;
        std::cout << filekey << endl;
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
            for(TagLib::StringList::ConstIterator j = i->second.begin(); j != i->second.end(); ++j) 
            {
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
}
    return 0;
}

int handle_file(const std::string& filepath, const std::string& filekey, AudioFileRecordStore& record_store)
{
    return handle_file(filepath.c_str(), filekey.c_str(), record_store);
}

int handle_directory(const char* path, const char* filekey, AudioFileRecordStore& record_store)
{
    return 1;
}

int handle_directory(const std::string& path, const std::string& filekey, AudioFileRecordStore& record_store)
{
    return handle_directory(path.c_str(), filekey.c_str(), record_store);
}

} //namespace
