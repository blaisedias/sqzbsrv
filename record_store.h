#ifndef RECORD_STORE_H_INCLUDED
#define RECORD_STORE_H_INCLUDED

#include <map>
#include "audio_file_tags.h"

namespace record_store {
// Serialisation support functions, are externalised, so that they can be serialised as required.
// These functions must be defined in the file where the record store is instantiated.
template <typename KeyType, typename RecordType> void fload(const char* location, std::map<KeyType, RecordType>&);
template <typename KeyType, typename RecordType> void fsave(const char* location, const std::map<KeyType, RecordType>&);
template <typename KeyType, typename RecordType> void ftest(const std::map<KeyType, RecordType>&);

// template implementing the AudioFileRecordStore interface.
// class KeyType must 1) implement function c_str() which returns null terminated const char * to the contents.
//                    2) meet serialisation requirements for fload and fsave function definitions.
//                    3) implement operator<(const char *) const
template <typename KeyType, typename RecordType>
class RecordStore:public audio_file_tags::AudioFileRecordStore
{
    private:
        // file records table
        std::map<KeyType, RecordType> records;
        // file records storage location
        std::string records_location;
    public:
        RecordStore():records_location("record_store.dat") {}
        RecordStore(std::string location):records_location(location) {}
        RecordStore(const char * location):records_location(location) {}

        //serialisation support
        void save()
        {
            fsave(records_location.c_str(), records);
        }

        void load()
        {
            fload(records_location.c_str(), records);
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
            if (records.find(location) == records.end())
                return true;
            return records[location].update_required();
        }

        bool record_update_required(const std::string &location)
        {
            return record_update_required(location.c_str());
        }

        void remove_record(const std::string &location)
        {
            records.erase(KeyType(location));
        }

        void remove_record(const char *location)
        {
            records.erase(KeyType(location));
        }

        // Enumerate all records and refresh them by invoking the supplied function.
        void refresh_records(int (*updatefn)(const char *, AudioFileRecordStore&))
        {
            for(typename std::map<KeyType, RecordType>::iterator itr=records.begin();
                    itr != records.end(); ++itr)
            {
//                RecordType &ti = itr->second;
                updatefn(itr->first.c_str(), *this);
            }
        }

        // debug helper function. FIXME: take an ostream for maximum flexibility.
        void dump_records()
        {
            for(typename std::map<KeyType, RecordType>::iterator itr=records.begin();
                    itr != records.end(); ++itr)
            {
                itr->second.dump();
            }
        }

        // debug helper function.
        void test()
        {
            ftest(records);
        }
};

}
#endif  //RECORD_STORE_H_INCLUDED
