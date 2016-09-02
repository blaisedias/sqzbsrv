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
#ifndef SSTRING_H_INCLUDED
#define SSTRING_H_INCLUDED
#include <iostream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace sstring {
// serialization environment interface.
class SerializationContext {
    protected:
        virtual ~SerializationContext(){};
    public:
        // returns if remapping of ids is required at deserialization time.
        virtual bool is_remapped()=0;
        // returns remapped id.
        virtual unsigned remapped_id(unsigned id)=0;
};

// String data database object interface.
class Registry {
    public:
        // load from a file, merges with existing content
        virtual void load(const char * filename, SerializationContext* psc)
        {
            std::ifstream ifs(filename);
            if (ifs.is_open())
                load(ifs, psc);
        }
        // load from a stream
        virtual void load(std::istream&, SerializationContext *)=0;

        // saves snapshot to file, can be called at any time.
        // If pruning is required it should be done before saving.
        virtual void save(const char* filename, SerializationContext* psc)
        {
            std::ofstream ofs(filename);
            if (ofs.is_open())
                save(ofs, psc);
        }
        // saves snapshot to a stream, can be called at any time.
        virtual void save(std::ostream&, SerializationContext*)=0;

        // removes unused strings, IDs are not currently recycled
        virtual void prune()=0;

        // serialization context, to support multiple
        // serialization to and deserialization ops from removable storage.
        virtual SerializationContext*  getSerializationContext()=0;
        virtual SerializationContext*  getDefaultSerializationContext()=0;

        // Debug helper function. TODO: give it a stream to dump to.
        virtual void dump()=0;

    protected:
        virtual ~Registry() {};
};

// Currently only support for a single default string data database.
Registry& getRegistry();

class String {
    private:
        unsigned id;

        inline void refup();
        inline void refdn();
        void bind(unsigned id);

        friend class boost::serialization::access;
        template <class Archive>
            void load(Archive &ar, const unsigned int version);
        template <class Archive>
            void save(Archive &ar, const unsigned int version) const;
        BOOST_SERIALIZATION_SPLIT_MEMBER();

    public:
        //CTORs
        String(const char * const chars);
        String(const std::string& strng);
        String(const char * const chars, SerializationContext *psc);
        String(const std::string& strng, SerializationContext *psc);
        String(const String &sstr);
        String(unsigned id=0);
        virtual ~String();

        String& operator=(const String& sstr);
        bool operator==(const String& sstr) const;
        bool operator==(const char * c_str) const;
        bool operator==(const std::string& cpp_str) const;

        bool operator<(const char *c_str) const;
        bool operator<(const String& sstr) const;

        bool operator>(const String& sstr) const;

        //Conversions
        inline operator unsigned() const { return id; }
        inline operator unsigned*() const { return nullptr; }
        inline operator const char*() const { return c_str(); }

        //Conversion functions
        std::string std_str() const;
        const char* c_str() const;

        friend std::ostream& operator<< (std::ostream& os, const String& sstr);

        //Hashing support
        std::size_t hash() const;

        //
        void setSerializationContext(SerializationContext *psc);
};

} // namespace sstring

namespace std
{
    template<>
    struct hash<sstring::String>
    {
        size_t operator()(sstring::String const& s) const
        {
            return s.hash();
        }
    };
} // namespace std

#endif
