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
#include <sstream>

#undef DEBUG

// for serialisation
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "fs_utils.h"
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

class Context
{
    public:
        virtual uintmax_t file_size(const std::string&)=0;
        virtual time_t file_timestamp(const std::string&)=0;
};

const static std::string unknown ="UNKNOWN";
class TrackInfoImpl : audio_file_tags::AudioFileRecord {
    private:
        uintmax_t file_length;
        time_t file_timestamp;
        bool complete;

        std::map<int, std::string> infomap;

        // Meh. allow container class access to my private parts!
        // so that the context pointer can be setup.
        friend class tracksRecordStore;
        Context *ctxt;

    public:
        TrackInfoImpl(){
#ifdef  DEBUG           
           std::cout << " TrackInfoImpl " << this << " " << &infomap << std::endl;
#endif
           ctxt = NULL;
        }

        TrackInfoImpl(const std::string& filename, Context *pcontext){
#ifdef  DEBUG           
           std::cout << " TrackInfoImpl " << this << " " << &infomap << std::endl;
#endif
           ctxt = pcontext;
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
            assert(ctxt != NULL);
            file_length = ctxt->file_size(filename);
            file_timestamp = ctxt->file_timestamp(filename);
            complete = false;
        }

        bool update_required()
        {
            assert(ctxt != NULL);
            try
            {
                return (complete != true)
                    || file_timestamp != ctxt->file_timestamp(infomap[0])
                    || file_length != ctxt->file_size(infomap[0]);
            }
            catch(...)
            {
                return true;
            }
        }

        void update_complete()
        {
            assert(ctxt != NULL);
            file_length = ctxt->file_size(infomap[0]);
            file_timestamp = ctxt->file_timestamp(infomap[0]);
            complete = true;
        }

        void update_start()
        {
            complete = false;
            int ix_fp_tag = audio_tags::index_of_supported(std::string(audio_tags::FILEPATH));

            for(std::map<int, std::string>::iterator itr=infomap.begin();
                itr != infomap.end(); ++itr)
            {
                if ((itr->first == ix_fp_tag))
                    continue;
                infomap.erase(itr->first);
            }
        }

        const std::string& get(int ix_tag) const
        {
            std::map<int, std::string>::const_iterator find = infomap.find(ix_tag);
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

class tracksRecordStore: public record_store::RecordStore<std::string, TrackInfoImpl>, public Context
{
    protected:
        void new_record(const char *location)
        {
            records.emplace(location, TrackInfoImpl(location, this));
        }
    public:
        tracksRecordStore(const char *location, const char *fname): record_store::RecordStore<std::string, TrackInfoImpl>(location, fname){}

        void deserialise_records(std::istream& ifs, std::unordered_map<std::string, TrackInfoImpl>& recs_in)
        {
            record_store::RecordStore<std::string, TrackInfoImpl>::deserialise_records(ifs, recs_in);
            for (auto itr=recs_in.begin(); itr != recs_in.end(); ++itr)
            {
                assert(itr->second.ctxt == NULL);
                itr->second.ctxt = this;
            }
        }

        uintmax_t file_size(const std::string& f)
        {
            std::string filename(rootdir);
            fs_utils::path_append(filename, f);
            return fs_utils::file_size(filename);
        }

        time_t file_timestamp(const std::string& f)
        {
            std::string filename(rootdir);
            fs_utils::path_append(filename, f);
            return fs_utils::file_timestamp(filename);
        }
};

audio_file_tags::AudioFileRecordStoreCollection* new_record_store_collection()
{
    return new record_store::RecordStoreCollection<tracksRecordStore>("tracks_db.dat"); 
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
        const RecordType& tii = (itr->second);
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

