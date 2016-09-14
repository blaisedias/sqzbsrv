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
#include <mutex>

#include "fs_utils.h"
#include "audio_file_tags.h"
#include "scanner.h"

namespace record_store {
// These functions must be defined in the file where the record store is instantiated.
template <typename KeyType, typename RecordType> void ftest(const std::unordered_map<KeyType, RecordType>&);

template <typename RecordStoreType>
class RecordStoreCollection : public audio_file_tags::AudioFileRecordStoreCollection {
    protected:
        std::unordered_map<std::string, RecordStoreType*> stores;
        std::string dbfilename;
    public:
        RecordStoreCollection(const char* f):dbfilename(f) {}
        ~RecordStoreCollection()
        {
            for (auto itr=stores.begin(); itr != stores.end(); ++itr)
            {
                RecordStoreType* rec_store = itr->second;
                delete rec_store;
            }
            stores.clear();
        }

        void save()
        {
            for (auto itr=stores.begin(); itr != stores.end(); ++itr)
            {
                RecordStoreType& rec_store = *itr->second;
                rec_store.save();
            }
        }

        void load(const char* rootdir)
        {
            if (stores.find(rootdir) == stores.end())
            {
                //stores.insert({std::string(rootdir), RecordStoreType(rootdir, dbfilename.c_str()}));
                stores[rootdir] = new RecordStoreType(rootdir, dbfilename.c_str());
            }

            RecordStoreType& rec_store = *stores[rootdir];
            rec_store.load();
        }

        void scan(const char* rootdir)
        {
            if (stores.find(rootdir) == stores.end())
            {
                //stores.insert({std::string(rootdir), RecordStoreType(rootdir, dbfilename.c_str()}));
                stores[rootdir] = new RecordStoreType(rootdir, dbfilename.c_str());
            }
            RecordStoreType& rec_store = *stores[rootdir];
            Scanner::Scanner scanner = Scanner::Scanner((audio_file_tags::AudioFileRecordStore&)rec_store);
            scanner.scan(rootdir);
        }

        void refresh_records()
        {
            for (auto itr=stores.begin(); itr != stores.end(); ++itr)
            {
                RecordStoreType& rec_store = *itr->second;
                rec_store.refresh_records();
            }
        }

        void test()
        {
            //FIXME: this doesn't work correctly with collections,
            //the output files are created and written to by the each
            //record store, and we want a cumulative output file.
            //Also the enumeration order has to match!
            //So enumerate the keys, sort the keys and then output.
            for (auto itr=stores.begin(); itr != stores.end(); ++itr)
            {
                RecordStoreType& rec_store = *itr->second;
                rec_store.test();
            }
        }

        void dump_records()
        {
            for (auto itr=stores.begin(); itr != stores.end(); ++itr)
            {
                RecordStoreType& rec_store = *itr->second;
                rec_store.dump_records();
            }
        }
};


// template implementing the AudioFileRecordStore interface.
// class KeyType must 1) implement function c_str() which returns null terminated const char * to the contents.
//                    2) meet serialisation requirements for fload and fsave function definitions.
//                    3) implement operator<(const char *) const
template <typename KeyType, typename RecordType>
class RecordStore:public audio_file_tags::AudioFileRecordStore
{
    private:
        friend RecordType;
    protected:
        // file records table
        std::unordered_map<KeyType, RecordType> records;
        // file records storage location
        std::string records_location;
        std::recursive_mutex mutex;
        virtual inline int cb_update(const KeyType& key)
        {
            std::string filepath(rootdir);
            fs_utils::path_append(filepath, key);
            return audio_file_tags::handle_file(filepath, key.c_str(), *this);
        }
        std::string rootdir;
        std::string datafile;
    public:
        RecordStore(const char * location, const char* fname):rootdir(location),datafile(fname)
        {
            records_location.assign(rootdir);
            records_location.append("/");
            records_location.append(fname);
        }

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

    protected:
        // delegate this to the impl class, removes CTOR requirements on RecordType class.
        virtual void new_record(const char* location)=0;

    public:
        audio_file_tags::AudioFileRecord& get_record(const char *location)
        {
            if (records.find(location) == records.end())
            {
                new_record(location);
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
            //FIXME: this doesn't work correctly for record stores in
            //collections.
            ftest(records);
        }
};

}
#endif  //RECORD_STORE_H_INCLUDED
