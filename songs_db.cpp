#include <map>
#include <iostream>
#include <memory>
#include <boost/filesystem.hpp>

// for serialisation
#include <fstream>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include "audio_tags.h"
#include "record_store.h"
#include "songs_db.h"
#include "sstring.h"

static bool debug=true;

namespace songs_db {
typedef std::map<sstring::String, sstring::String> INFOMAP;

//static sstring::String fp_tag(audio_tags::FILEPATH);
//static sstring::String dir_tag(audio_tags::DIRECTORY);
static sstring::String fp_tag;
static sstring::String dir_tag;
static sstring::String unknown;

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

    if ((cmp > 0) && ((ix_match + 1) < len))
        ix_match++;        // last comparison => target > probe, so bump up
    *ixtop = ix_match;
    return cmp==0;
}

//const static sstring::String xx_fp_tag = audio_tags::FILEPATH;
static bool initialised=false;
class SongInfo : audio_file_tags::AudioFileRecord {
    private:
        uintmax_t file_length;
        time_t file_timestamp;
        bool complete;
        INFOMAP infomap;
        void init(const sstring::String& filename);
        void initS();

    public:
        SongInfo(){initS();}
        SongInfo(const std::string& filename);
        SongInfo(const sstring::String& filename);
        ~SongInfo(){}

        template <class Archive>
        void serialize(Archive &ar,  const unsigned int version)
        {
          ar & complete;
          ar & file_length;
          ar & file_timestamp;
          ar & infomap;
        }

        void update(const std::string tag, const std::string value);
        void update(const char * tag, const char *value);
        void update(const char *, int);
        void update(const std::string tag, int);
        void update_start();
        void update_complete();
        bool update_required();
        void dump();

        sstring::String get(sstring::String tag)
        {
            INFOMAP::iterator find = infomap.find(tag);
            if (find == infomap.end())
                return unknown;
            return find->second;
        }
};

void SongInfo::initS()
{
    if (initialised)
        return;

//    std::cout << "initS {{{{{{{{  " << std::endl;
//    fp_tag = sstring::String(audio_tags::FILEPATH);
//    dir_tag = sstring::String(audio_tags::DIRECTORY);
 //   unknown = sstring::String("UNKNOWN");
    fp_tag = audio_tags::FILEPATH;
    dir_tag = audio_tags::DIRECTORY;
    unknown = "UNKNOWN";
//    std::cout << "}}}}}}}}" << std::endl;
    initialised = true;
}

void SongInfo::init(const sstring::String& filename)
{
    initS();
    infomap[fp_tag] = filename;
    infomap[dir_tag] = sstring::String(boost::filesystem::path(filename.std_str()).parent_path().generic_string());
    file_length = boost::filesystem::file_size(filename.std_str());
    file_timestamp = boost::filesystem::last_write_time(filename.std_str());
    complete = false;
}

SongInfo::SongInfo(const sstring::String& filename)
{
    init(filename);
}

SongInfo::SongInfo(const std::string& filename)
{
    init(sstring::String(filename));
}

void SongInfo::update(const std::string tag, const std::string value)
{
    if(!audio_tags::is_supported(tag))
        return;
//    std::cout << "Update {{{{{{{{  " << tag << " = " << value << std::endl;
//    infomap[sstring::String(tag)] = sstring::String(value);
//    infomap[tag] = value;
    infomap.emplace(tag, value);
//    std::cout << "}}}}}}}}" << std::endl;
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
    for(INFOMAP::iterator itr=infomap.begin();
            itr != infomap.end(); ++itr)
    {
        if ((itr->first == fp_tag) || (itr->first == dir_tag))
            continue;
        infomap.erase(itr->first);
    }
}

void SongInfo::update_complete()
{
    file_length = boost::filesystem::file_size(infomap[fp_tag].std_str());
    file_timestamp = boost::filesystem::last_write_time(infomap[fp_tag].std_str());
    complete = true;
}

bool SongInfo::update_required()
{
    return (complete != true)
        || file_timestamp != boost::filesystem::last_write_time(infomap[fp_tag].std_str())
        || file_length != boost::filesystem::file_size(infomap[fp_tag].std_str());
}

void SongInfo::dump()
{
    std::cout << "Complete :" << complete << " Len: " << file_length  << " mtime:"  << file_timestamp <<  std::endl;
    for(INFOMAP::iterator itr2=infomap.begin(); itr2 != infomap.end(); ++itr2)
    {
        std::cout << itr2->first << " : " << itr2->second << std::endl;
    }
    std::cout << std::endl;
}

//audio_file_tags::AudioFileRecordStore* new_record_store(const char *database_location, const char *string_defs_file)
audio_file_tags::AudioFileRecordStore* new_record_store()
{
    return new record_store::RecordStore<sstring::String, SongInfo>("data/songs_db.dat");
}

} //namespace songs_db

namespace record_store {
template <typename KeyType, typename RecordType> void fsave(const char* filename, const std::map<KeyType, RecordType>& records)
{
    {
        std::ofstream ofs(filename);
        boost::archive::text_oarchive ar(ofs);
        ar << records;
    }
}

template <typename KeyType, typename RecordType> void fload(const char* filename, std::map<KeyType, RecordType>& records)
{
        std::ifstream ifs(filename);
        boost::archive::text_iarchive ar(ifs);
        ar >> records;
}

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

template <typename KeyType, typename RecordType> void ftest(const std::map<KeyType, RecordType>& records)
{
    static const sstring::String artist_tag(audio_tags::ARTIST);
    static const sstring::String album_tag(audio_tags::ALBUM);
    static const sstring::String title_tag(audio_tags::TITLE);
    static const sstring::String genre_tag(audio_tags::GENRE);

    std::vector<KeyType> artists;
    std::vector<KeyType> titles;
    std::vector<KeyType> albums;
    std::vector<KeyType> genres;
    for(typename std::map<KeyType, RecordType>::const_iterator itr=records.begin();
            itr != records.end(); ++itr)
    {
        RecordType& tii = const_cast<RecordType&>(itr->second);
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

