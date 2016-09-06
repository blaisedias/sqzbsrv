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
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <boost/filesystem.hpp>
#include <sstream>

#undef DEBUG

// for serialisation
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "audio_tags.h"
#include "record_store.h"
#include "tracks_db.h"

namespace tracks_db {

static void save_vec(const char *filename, std::vector<std::string> svec)
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

const static std::string unknown ="UNKNOWN";
class TrackInfoImpl : audio_file_tags::AudioFileRecord {
    private:
        std::map<int, std::string> infomap;
        uintmax_t file_length;
        time_t file_timestamp;
        bool complete;

    public:
        TrackInfoImpl(){
#ifdef  DEBUG           
           std::cout << " TrackInfoImpl " << this << " " << &infomap << std::endl;
#endif
        }

        TrackInfoImpl(const std::string& filename){
#ifdef  DEBUG           
           std::cout << " TrackInfoImpl " << this << " " << &infomap << std::endl;
#endif
           init(filename);
        }

        ~TrackInfoImpl(){
#ifdef  DEBUG           
            std::cout <<  " ~TrackInfoImpl " << this  << " " << &infomap << std::endl;
#endif
        }

        template <class Archive>
        void serialize(Archive &ar,  const unsigned int version)
        {
          ar & complete;
          ar & file_length;
          ar & file_timestamp;
          ar & infomap;
        } 

        void update(const std::string tag, const std::string value)
        {
            int ixtag = -1;
            //if (supported_tags_only)
            if (true)
            {
               if (!audio_tags::is_supported(tag))
                   return;
               ixtag = audio_tags::index_of_supported(tag);
            }
            else
            {
                if (audio_tags::is_unsupported(tag))
                    return;
                ixtag = audio_tags::index_of_supported(tag);
                
                if (ixtag < 0)     // tag not supported or unsupported so add tag
                    ixtag = audio_tags::extend_supported(tag);
            }

#ifdef  DEBUG           
            std::cout << "pimpl update " << this  << " " << ixtag << " " << value << std::endl;
#endif
            infomap.emplace(ixtag, value);
#ifdef  DEBUG           
            std::cout << "pimpl update emplace done." << std::endl;
#endif
            complete = false;
        }

        void update(const char* tag, const char* value)
        {
            return update(std::string(tag), std::string(value));
        }

        void update(const std::string tag, int value)
        {
            return update(tag,
                    dynamic_cast<std::ostringstream &>((std::ostringstream() << std::dec << value)).str());
        }

        void update(const char* tag, int value)
        {
            return update(std::string(tag), value);
        }

        void init(const std::string &filename)
        {
            update(std::string("FILEPATH"), filename);
            update(std::string("DIRECTORY"), boost::filesystem::path(filename).parent_path().generic_string());
            file_length = boost::filesystem::file_size(filename);
            file_timestamp = boost::filesystem::last_write_time(filename);
            complete = false;
        }

        bool update_required()
        {
            return (complete != true)
                || file_timestamp != boost::filesystem::last_write_time(infomap[0])
                || file_length != boost::filesystem::file_size(infomap[0]);
        }

        void update_complete()
        {
            file_length = boost::filesystem::file_size(infomap[0]);
            file_timestamp = boost::filesystem::last_write_time(infomap[0]);
            complete = true;
        }

        void update_start()
        {
            complete = false;
            int ix_fp_tag = audio_tags::index_of_supported(std::string(audio_tags::FILEPATH));
            int ix_dir_tag = audio_tags::index_of_supported(std::string(audio_tags::DIRECTORY));

            for(std::map<int, std::string>::iterator itr=infomap.begin();
                itr != infomap.end(); ++itr)
            {
                if ((itr->first == ix_fp_tag) || (itr->first == ix_dir_tag))
                    continue;
                infomap.erase(itr->first);
            }
        }

        const std::string& get(int ix_tag)
        {
            std::map<int, std::string>::iterator find = infomap.find(ix_tag);
            if (find == infomap.end())
                return unknown;
            return find->second;
        }

#if 0
        uintmax_t get_file_length()
        {
            return file_length;
        }

        time_t get_file_timestamp()
        {
            return file_timestamp;
        }

        bool get_complete()
        {
            return complete;
        }
#endif

        void dump()
        {
                std::map<std::string, std::string> smap;
                std::cout << "Complete :" << complete << " Len: " << file_length  << " mtime:"  << file_timestamp <<  std::endl;
                for(std::map<int, std::string>::iterator itr=infomap.begin();
                        itr != infomap.end(); ++itr)
                {
                    smap[audio_tags::value_of_supported(itr->first)] = itr->second;
//                    std::cout << itr->first << ") " << audio_tags::value_of_supported(itr->first) << ": " << itr->second << std::endl;
                }
                for(std::map<std::string, std::string>::iterator itr=smap.begin();
                        itr != smap.end(); ++itr)
                {
                    std::cout << itr->first << " : " << itr->second << std::endl;
                }
                std::cout << std::endl;
        }
};

audio_file_tags::AudioFileRecordStore* new_record_store()
{
    return new record_store::RecordStore<std::string, TrackInfoImpl>("./data", "tracks_db.dat");
}

} // namespace tracks_db


namespace record_store {
template <typename KeyType, typename RecordType> void ftest(const std::unordered_map<KeyType, RecordType>& records)
{
    std::vector<KeyType> artists;
    std::vector<KeyType> titles;
    std::vector<KeyType> albums;
    std::vector<KeyType> genres;

    int ix_artist = audio_tags::index_of_supported(std::string(audio_tags::ARTIST));
    int ix_album = audio_tags::index_of_supported(std::string(audio_tags::ALBUM));
    int ix_title = audio_tags::index_of_supported(std::string(audio_tags::TITLE));
    int ix_genre = audio_tags::index_of_supported(std::string(audio_tags::GENRE));
    for(typename std::unordered_map<KeyType, RecordType>::const_iterator itr=records.begin();
            itr != records.end(); ++itr)
    {
        // FIXME: Naughty casting away constness
        // cause I don't know how to fix this correctly (yet) given that we have const records.
        // Making get a const method fails compilation with
        // error: passing ‘const std::map<int, std::basic_string<char> >’ as ‘this’ argument of ‘std::map<_Key, _Tp, _Compare, _Alloc>::mapped_type& std::map<_Key, _Tp, _Compare, _Alloc>::operator[](const key_type&) [with _Key = int; _Tp = std::basic_string<char>; _Compare = std::less<int>; _Alloc = std::allocator<std::pair<const int, std::basic_string<char> > >; std::map<_Key, _Tp, _Compare, _Alloc>::mapped_type = std::basic_string<char>; std::map<_Key, _Tp, _Compare, _Alloc>::key_type = int]’ discards qualifiers [-fpermissive]
        // return infomap[ix_tag];
        RecordType& tii = const_cast<RecordType&>(itr->second);
        artists.push_back(tii.get(ix_artist));
        albums.push_back(tii.get(ix_album));
        titles.push_back(tii.get(ix_title));
        genres.push_back(tii.get(ix_genre));
    }
    tracks_db::save_vec("txt/artists.txt", artists);
    tracks_db::save_vec("txt/albums.txt", albums);
    tracks_db::save_vec("txt/titles.txt", titles);
    tracks_db::save_vec("txt/genres.txt", genres);
}

} // namespace record_store

