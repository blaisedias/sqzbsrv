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
#include <iostream>
#include <memory>
#include <unordered_map>
#include <map>

#include <string.h>

// for serialisation
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include "fs_utils.h"
#include "audio_tags.h"
#include "record_store.h"
#include "songs_db.h"
#include "sstring.h"

static bool debug=true;

namespace songs_db {
typedef std::unordered_multimap<sstring::String, sstring::String> INFOMAP;
typedef std::pair<sstring::String, sstring::String> INFOMAP_PAIR;

const char* const unknown="UNKNOWN";

// Binary search for string on sorted vectors
// returns success or failure, + slot where string should be inserted.
template<typename T> int Bsearch(T strings, int len, const char *target, int *ixtop)
{
    int start=0;
    int end = len;
    int ix_match = len -1;

    int cmp;
    int probe;
    const char *sprobe;
    while(start <= end && start < len)
    {
        probe = start + ((end-start)/2);
        sprobe = strings[probe].c_str();

        ix_match = probe;
        cmp = strcmp(target, sprobe);
        if (debug)
            printf("\t\t\t %d - %d %d) cmp=%d  %s == %s\n", start, end, probe, cmp, target, sprobe);
        if (cmp == 0)
            break;

        if(cmp < 0)
        {
            end = probe - 1;
        }
        else
        {
            start = probe + 1;
        }
    }

    if (cmp > 0)
    {
        // at last comparison target > probe, so point to the next slot.
        ix_match++;
    }
    *ixtop = ix_match;
    return cmp==0;
}

class Context
{
    public:
        virtual uintmax_t file_size(const char*)=0;
        virtual time_t file_timestamp(const char*)=0;
        virtual sstring::SerializationContext *serialization_context()=0;
};

// construct a sstring::String in a serialized context
#define SSSTRING(str)    sstring::String((str), ctxt->serialization_context())

class SongInfo : audio_file_tags::AudioFileRecord {
    private:
        uintmax_t file_length;
        time_t file_timestamp;
        bool complete;

        INFOMAP infomap;
        void init(const sstring::String& filename);

        // Meh. allow container class access to my private parts!
        // so that the context pointer can be setup.
        friend class songsRecordStore;
        Context* ctxt;
    public:
        SongInfo()
        {
            ctxt = NULL;
        }
        SongInfo(const char* const filename, Context* pcontext)
        {
            ctxt = pcontext;
            init(SSSTRING(filename));
        }
        ~SongInfo(){}

        template <class Archive>
        void save(Archive &ar,  const unsigned int version) const
        {
          ar & complete;
          ar & file_length;
          ar & file_timestamp;
          std::size_t infomap_len = infomap.size();
          ar & infomap_len;
          for(INFOMAP::const_iterator itr2=infomap.begin(); itr2 != infomap.end(); ++itr2)
          {
              ar & itr2->first;
              ar & itr2->second;
          }
        }
        template <class Archive>
        void load(Archive &ar,  const unsigned int version)
        {
          ar & complete;
          ar & file_length;
          ar & file_timestamp;
          std::size_t infomap_len;
          ar & infomap_len;
          while(infomap_len--)
          {
              sstring::String key;
              sstring::String value;
              ar & key;
              ar & value;
              infomap.emplace(key, value);
          }
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER();

        void update(const std::string tag, const std::string value);
        void update(const char * tag, const char *value);
        void update(const char *, int);
        void update(const std::string tag, int);
        void update_start();
        void update_complete();
        bool update_required();
        void dump();

        const sstring::String get(sstring::String tag) const
        {
            INFOMAP::const_iterator find = infomap.find(tag);
            if (find == infomap.end())
                return unknown;
            return find->second;
        }
};

void SongInfo::init(const sstring::String& filename)
{
//    infomap.insert(INFOMAP_PAIR(SSSTRING(audio_tags::FILEPATH),SSSTRING(filename));
    infomap.emplace(SSSTRING(audio_tags::FILEPATH), SSSTRING(filename));
    assert(ctxt != NULL);
    file_length = ctxt->file_size(filename);
    file_timestamp = ctxt->file_timestamp(filename);
    complete = false;
}

void SongInfo::update(const std::string tag, const std::string value)
{
    if(!audio_tags::is_supported(tag))
        return;
    bool duplicate = false;
    auto range = infomap.equal_range(tag);
    for (auto it=range.first; it != range.second; ++it)
    {
        if (it->second == value)
            duplicate = true;
    }
    if (!duplicate)
        infomap.emplace(SSSTRING(tag), SSSTRING(value));
    complete = false;
}

void SongInfo::update(const char * tag, const char *value)
{
    return update(std::string(tag), std::string(value));
}

void SongInfo::update(const char *tag, int value)
{
#if 0    
    static const char* hexchars="0123456789ABCDEF";
    int ix = sizeof(value)*2;
    char value_chars[ix + 1];
    value_chars[ix--] = 0;
    unsigned int v = (unsigned int) value;
    while(ix >= 0)
    {
        value_chars[ix] = hexchars[(v & 0xf)];
        v >>= 4;
        ix--;
    }
    update(std::string(tag), std::string(value_chars));
#else
    return update(tag,
            dynamic_cast<std::ostringstream &>((std::ostringstream() << std::dec << value)).str());
#endif
}

void SongInfo::update(const std::string tag, int value)
{
    update(tag.c_str(), value);
}

void SongInfo::update_start()
{
    complete = false;
    std::vector<sstring::String> v;
    v.reserve(infomap.size());
    for(INFOMAP::iterator itr=infomap.begin(); itr != infomap.end(); ++itr)
    {
        if ((itr->first == audio_tags::FILEPATH) || (itr->first == audio_tags::DIRECTORY))
            continue;
        v.push_back(itr->first);
    }

    for(auto x: v)
    {
        infomap.erase(x);
    }
}

void SongInfo::update_complete()
{
    auto find = infomap.find(audio_tags::FILEPATH);
    assert(find != infomap.end());
    assert(ctxt != NULL);
    file_length = ctxt->file_size(find->second);
    file_timestamp = ctxt->file_timestamp(find->second);
    complete = true;
}

bool SongInfo::update_required()
{
    assert(ctxt != NULL);
    if (complete)
    {
        try
        {
            auto find = infomap.find(audio_tags::FILEPATH);
            return file_timestamp != ctxt->file_timestamp(find->second)
                || file_length != ctxt->file_size(find->second);
        }
        catch(...)
        {
        }
    }
    return true;
}

void SongInfo::dump()
{
    std::cout << "Complete :" << complete << " Len: " << file_length  << " mtime:"  << file_timestamp <<  std::endl;
#if 0
    typedef std::map<sstring::String, sstring::String> SORTEDINFOMAP;
    SORTEDINFOMAP smap;
    for(INFOMAP::iterator itr=infomap.begin(); itr != infomap.end(); ++itr)
    {
        smap.emplace(itr->first, itr->second);
    }

    for(SORTEDINFOMAP::iterator itr=smap.begin(); itr != smap.end(); ++itr)
    {
        std::cout << itr->first << " : " << itr->second << std::endl;
    }
#else
    std::vector<sstring::String> v;
    v.reserve(infomap.size());
    for(INFOMAP::iterator itr=infomap.begin(); itr != infomap.end(); ++itr)
        v.push_back(itr->first);
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    for(auto n: v)
    {
        auto range = infomap.equal_range(n);
        for (auto it = range.first; it != range.second; ++it)
        {
            std::cout << n << " : " << it->second << "\n";
        }
    }
#endif
    std::cout << std::endl;

}

class songsRecordStore: public record_store::RecordStore<sstring::String, SongInfo>, public Context
{
    private:
        std::vector<sstring::String> fixed_strings_list;
        sstring::SerializationContext *serialization_ctxt;
    protected:
        virtual inline int cb_update(const sstring::String& key)
        {
            std::string filepath(rootdir);
            fs_utils::path_append(filepath, key);
            return audio_file_tags::handle_file(filepath, key.c_str(), *this); 
        }
    public:
        songsRecordStore(const char *location, const char *fname): record_store::RecordStore<sstring::String, SongInfo>(location, fname)
        {
            serialization_ctxt = sstring::getRegistry().makeSerializationContext();
            std::vector <std::string> tag_strings;
            audio_tags::get_supported_taglist(tag_strings);
            fixed_strings_list.push_back(unknown);
            for(auto ts : tag_strings)
                fixed_strings_list.emplace_back(sstring::String(ts, serialization_ctxt));
        }

        void serialise_records(std::ostream& ofs, const std::unordered_map<sstring::String, SongInfo>& recs_out)
        {
            sstring::getRegistry().prune();
            sstring::getRegistry().save(ofs, serialization_ctxt);
            if (debug)
                std::cerr << " cchars written" << std::endl;
            record_store::RecordStore<sstring::String, SongInfo>::serialise_records(ofs, recs_out);
        }

        void deserialise_records(std::istream& ifs, std::unordered_map<sstring::String, SongInfo>& recs_in)
        {
            sstring::getRegistry().load(ifs, serialization_ctxt);
            if (debug)
                std::cerr << " cchars loaded" << std::endl;
            record_store::RecordStore<sstring::String, SongInfo>::deserialise_records(ifs, recs_in);
            for (auto itr=recs_in.begin(); itr != recs_in.end(); ++itr)
            {
                assert(itr->second.ctxt == NULL);
                itr->second.ctxt = this;
            }
        }

        inline const audio_file_tags::AudioFileRecord* const find_record(const char *location)
        {
            if (sstring::getRegistry().exists(location))
                return record_store::RecordStore<sstring::String, SongInfo>::find_record(location);
            return NULL;
        }

        void new_record(const char *location)
        {
            records.emplace(
                    sstring::String(location, serialization_ctxt),
                    SongInfo(sstring::String(location, serialization_ctxt),
                                this));
        }

        uintmax_t file_size(const char* f)
        {
            std::string filename(rootdir);
            fs_utils::path_append(filename, f);
            return fs_utils::file_size(filename);
        }

        time_t file_timestamp(const char* f)
        {
            std::string filename(rootdir);
            fs_utils::path_append(filename, f);
            return fs_utils::file_timestamp(filename);
        }

        inline sstring::SerializationContext *serialization_context()
        {
            return serialization_ctxt;
        }
};

audio_file_tags::AudioFileRecordStoreCollection* new_record_store_collection()
{
    return new record_store::RecordStoreCollection<songsRecordStore>("songs_db.dat"); 
}

} //namespace songs_db

namespace record_store {
template <typename KeyType> static void save_vec(const char *filename, std::vector<KeyType> svec)
{
    std::sort(svec.begin(), svec.end());
    svec.erase(std::unique(svec.begin(), svec.end()), svec.end());
    std::ofstream ofs(filename);
    if(ofs.is_open())
    {
        for (unsigned ix=0; ix < svec.size(); ix++)
        {
            ofs << svec[ix] << std::endl;
        }
    }
}

template <typename KeyType, typename RecordType> void ftest(const std::unordered_map<KeyType, RecordType>& records)
{
    static const sstring::String artist_tag(audio_tags::ARTIST);
    static const sstring::String album_tag(audio_tags::ALBUM);
    static const sstring::String title_tag(audio_tags::TITLE);
    static const sstring::String genre_tag(audio_tags::GENRE);

    std::vector<KeyType> artists;
    std::vector<KeyType> titles;
    std::vector<KeyType> albums;
    std::vector<KeyType> genres;
    for(typename std::unordered_map<KeyType, RecordType>::const_iterator itr=records.begin();
            itr != records.end(); ++itr)
    {
        const RecordType& tii = itr->second;
        artists.push_back(tii.get(artist_tag));
        albums.push_back(tii.get(album_tag));
        titles.push_back(tii.get(title_tag));
        genres.push_back(tii.get(genre_tag));
    }
    save_vec("txt/s_artists.txt", artists);
    save_vec("txt/s_albums.txt", albums);
    save_vec("txt/s_titles.txt", titles);
    save_vec("txt/s_genres.txt", genres);
}

} // namespace record_store

