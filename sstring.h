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
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace sstring {

class String {
    private:
        unsigned id;

        void bind(unsigned id);
        friend class boost::serialization::access;
        template <class Archive>
            void load(Archive &ar, const unsigned int version);
        template <class Archive>
            void save(Archive &ar, const unsigned int version) const;
        BOOST_SERIALIZATION_SPLIT_MEMBER();

        inline void refup();
        inline void refdn();
    public:
        String(const char * const chars);
        String(const String &sstr);
        String(const std::string& strng);
        String(unsigned id=0);
        virtual ~String();

        String& operator=(const String& sstr);
        bool operator==(const String& sstr) const;
        bool operator==(const char * c_str) const;
        bool operator==(const std::string& cpp_str) const;
        bool operator<(const String& sstr) const;
        bool operator<(const char *c_str) const;

        std::string std_str() const;
        const char* c_str() const;
        friend std::ostream& operator<< (std::ostream& os, const String& sstr);
        std::size_t hash() const;
};

// String data database object interface.
class Registry {
    public:
        virtual void load(const char * filename)=0;
        virtual void save(const char * filename)=0;
        virtual void prune()=0;
        virtual void dump()=0;
        virtual ~Registry() {};
};

// Currently only support for a single default string data database.
Registry& getRegistry();

} // sstring

namespace std
{
    template<>
    struct hash<sstring::String>
    {
        typedef sstring::String argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& s) const
        {
//            return  std::hash<std::string>()(s.std_str());
            return  s.hash();
        }
    };
}

#endif
