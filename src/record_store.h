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
#ifndef RECORD_STORE_H_INCLUDED
#define RECORD_STORE_H_INCLUDED

#include <unordered_map>
#include "audio_file_tags.h"
#include <mutex>

namespace record_store {
// These functions must be defined in the file where the record store is instantiated.
template <typename KeyType, typename RecordType> void ftest(const std::unordered_map<KeyType, RecordType>&);

// template implementing the AudioFileRecordStore interface.
// class KeyType must 1) implement function c_str() which returns null terminated const char * to the contents.
//                    2) meet serialisation requirements for fload and fsave function definitions.
//                    3) implement operator<(const char *) const
template <typename KeyType, typename RecordType>
class RecordStore:public audio_file_tags::AudioFileRecordStore
{
    private:
        // file records table
        std::unordered_map<KeyType, RecordType> records;
        // file records storage location
        std::string records_location;
        std::recursive_mutex   mutex;
    protected:
        virtual inline int cb_update(const KeyType& key)
        {
            return audio_file_tags::handle_file(key.c_str(), *this);
        }
    public:
        RecordStore():records_location("record_store.dat") {}
        RecordStore(std::string location):records_location(location) {}
        RecordStore(const char * location):records_location(location) {}
        virtual ~RecordStore() {}

        //serialisation support
        void save() final
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            std::ofstream ofs(records_location.c_str());
            serialise(ofs);
        }

        virtual void serialise(std::ostream& ofs) final
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            serialise_records(ofs, records);
        }

        virtual void serialise_records(std::ostream& ofs, const std::unordered_map<KeyType, RecordType>& recs_out)
        {
            boost::archive::text_oarchive ar(ofs);
            std::size_t num_records=recs_out.size();
            ar << num_records;
            for(typename std::unordered_map<KeyType, RecordType>::const_iterator itr=recs_out.begin();
                    itr != recs_out.end(); ++itr)
            {
                ar << itr->first;
                ar << itr->second;
            }
        }

        void load() final
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            std::ifstream ifs(records_location.c_str());
            deserialise(ifs);
        }

        virtual void deserialise(std::istream& ifs) final
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            deserialise_records(ifs, records);
        }

        virtual void deserialise_records(std::istream& ifs, std::unordered_map<KeyType, RecordType>& recs_in)
        {
            boost::archive::text_iarchive ar(ifs);
            std::size_t num_records;
            ar >> num_records;
            while(num_records--)
            {
                KeyType key;
                RecordType rec;
                ar >> key;
                ar >> rec;
                recs_in.emplace(key,rec);
            }
        }

        inline const audio_file_tags::AudioFileRecord* const find_record(const char *location)
        {
            if (records.find(location) != records.end())
                return &((audio_file_tags::AudioFileRecord&)(records[location]));
            return NULL;
        }

        inline audio_file_tags::AudioFileRecord& record_end(void)
        {
            return records.end();
        }

        audio_file_tags::AudioFileRecord& get_record(const char *location)
        {
            if (records.find(location) == records.end())
            {
                KeyType kt_location(location);
                records[kt_location] = RecordType(kt_location);
            }
            return (audio_file_tags::AudioFileRecord&)(records[location]);
        }

        audio_file_tags::AudioFileRecord& get_record(const std::string& location)
        {
            return get_record(location.c_str());
        }

        bool record_update_required(const char *location)
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            if (records.find(location) == records.end())
                return true;
            return records[location].update_required();
        }

        bool record_update_required(const std::string &location)
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            return record_update_required(location.c_str());
        }

        void remove_record(const std::string &location)
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            records.erase(KeyType(location));
        }

        void remove_record(const char *location)
        {
            std::lock_guard<std::recursive_mutex> lock{mutex};
            records.erase(KeyType(location));
        }

        // Enumerate all records and refresh them by invoking the supplied function.
        void refresh_records()
        {
            for(typename std::unordered_map<KeyType, RecordType>::iterator itr=records.begin();
                    itr != records.end(); ++itr)
            {
                cb_update(itr->first);
            }
        }

        // debug helper function. TODO: take an ostream for maximum flexibility.
        void dump_records()
        {
#if 0
            for(typename std::unordered_map<KeyType, RecordType>::iterator itr=records.begin();
                    itr != records.end(); ++itr)
            {
                itr->second.dump();
            }
#endif
            std::vector<KeyType> v;
            v.reserve(records.size());
            for(typename std::unordered_map<KeyType, RecordType>::iterator itr=records.begin();
                    itr != records.end(); ++itr)
            {
                v.push_back(itr->first);
            }
            std::sort(v.begin(), v.end());
            for(auto n: v)
                records[n].dump();
        }

        // debug helper function.
        void test()
        {
            ftest(records);
        }
};

}
#endif  //RECORD_STORE_H_INCLUDED
