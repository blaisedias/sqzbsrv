#include <vector>
#include <map>
#include <fstream>

#include "audio_tags.h"
namespace audio_tags
{
    const char * FILEPATH = "FILEPATH";
    const char * DIRECTORY = "DIRECTORY";
    const char * FILELENGTH = "FILELENGTH";
    const char * FILETIMESTAMP = "FILETIMESTAMP";

    const char * BITRATE = "BITRATE";
    const char * LENGTH = "LENGTH";
    const char * SAMPLERATE = "SAMPLERATE";
    const char * CHANNELS = "CHANNELS";
    const char * ALBUM = "ALBUM";
    const char * ARTIST = "ARTIST";
    const char * TITLE = "TITLE";
    const char * DATE = "DATE";
    const char * TRACKNUMBER = "TRACKNUMBER";
    const char * DISCNUMBER = "DISCNUMBER";
    const char * ALBUMARTIST = "ALBUMARTIST";
    const char * GENRE = "GENRE";
    const char * FILETYPE = "FILETYPE";
    const char * COMPOSER = "COMPOSER";
    const char * CONDUCTOR = "CONDUCTOR";

static bool supported_tags_only = true;
static bool supported_tags_write_required = false;
static std::vector<std::string> supported_tags;
static std::map<std::string, int> supported_tags_map;

static bool unsupported_tags_write_required = false;
static std::vector<std::string> unsupported_tags;
static std::map<std::string, int> unsupported_tags_map;
    
static const char *supported_tags_filename  = "data/supported_tags.dat";
static const char *unsupported_tags_filename  = "data/unsupported_tags.dat";

class StaticInitializer {
    public:
        StaticInitializer ()
        {
            const char * const ctags[] = 
            {
                audio_tags::FILEPATH,
                audio_tags::DIRECTORY,
                audio_tags::FILELENGTH,
                audio_tags::FILETIMESTAMP,
                audio_tags::BITRATE,
                audio_tags::LENGTH,
                audio_tags::SAMPLERATE,
                audio_tags::CHANNELS,
                audio_tags::ALBUM,
                audio_tags::ARTIST,
                audio_tags::TITLE,
                audio_tags::DATE,
                audio_tags::TRACKNUMBER,
                audio_tags::DISCNUMBER,
                audio_tags::ALBUMARTIST,
                audio_tags::GENRE,
//                audio_tags::FILETYPE,
                audio_tags::COMPOSER,
                audio_tags::CONDUCTOR,
            };
            for (unsigned ix =0; ix < (sizeof(ctags)/sizeof(*ctags)); ix ++)
            {
                supported_tags.push_back(std::string(ctags[ix]));
                supported_tags_map[supported_tags[ix]] = ix;
            }

            unsupported_tags.push_back(std::string("*"));
            unsupported_tags_map[unsupported_tags[0]] = 0;
            supported_tags_only = true;
        }
};

static StaticInitializer static_init;
static void fsave_tags(const char *tags_filename, const std::vector<std::string>& tags, const std::map<std::string, int>& tags_map)
{
    std::ofstream ofs(tags_filename);
    for (unsigned ix=0; ix < tags.size(); ix++)
    {
        ofs << tags[ix] << std::endl;
    }
}

void save()
{
    if (supported_tags_write_required)
        fsave_tags(supported_tags_filename, supported_tags, supported_tags_map);
    supported_tags_write_required = false;
    if (supported_tags_write_required)
        fsave_tags(unsupported_tags_filename, unsupported_tags, unsupported_tags_map);
    unsupported_tags_write_required = false;
}

static void fload_tags(const char *tags_filename, std::vector<std::string>& tags, std::map<std::string, int>& tags_map)
{
//  FIXME: inplace loading of tags, maybe better to read to a.n.other and copy to file variables
    std::ifstream ifs(tags_filename);
    if (ifs.is_open())
    {
        tags.clear();
        tags_map.clear();
        char line[100];
        for (; ifs.getline(line, 100-1); )
            tags.push_back(std::string(line));
        for (unsigned ix=0; ix < tags.size(); ix ++)
        {
            tags_map[tags[ix]] = ix;
        }
    }
}

void load()
{
    fload_tags(supported_tags_filename, supported_tags, supported_tags_map);
    fload_tags(unsupported_tags_filename, unsupported_tags, unsupported_tags_map);
    supported_tags_only = 0 != unsupported_tags_map.count(std::string("*"));
}

void dump()
{
            for(unsigned ix=0; ix < supported_tags.size(); ix++)
                std::cout << ix << ")" << supported_tags[ix]  << std::endl;
}


bool is_supported(const std::string& tag)
{
    return supported_tags_map.count(tag) != 0;
}

bool is_unsupported(const std::string& tag)
{
    return unsupported_tags_map.count(tag) != 0;
}

int index_of_supported(const std::string& tag)
{
    if(supported_tags_map.count(tag))
        return supported_tags_map[tag];
    return -1;
}

const std::string& value_of_supported(int ix)
{
    return supported_tags[ix];
}

int extend_supported_tag(const std::string& tag)
{
    int ixtag = supported_tags.size();
    supported_tags.emplace_back(tag);
//    supported_tags.push_back(tag);
    supported_tags_map[tag] = ixtag;
    supported_tags_write_required = true;
    return ixtag;
}

void get_supported_taglist(std::vector<std::string> &v)
{
    std::copy(supported_tags.begin(), supported_tags.end(), std::back_inserter(v));
}

void get_unsupported_taglist(std::vector<std::string> &v)
{
    std::copy(unsupported_tags.begin(), unsupported_tags.end(), std::back_inserter(v));
}

} //namespace
