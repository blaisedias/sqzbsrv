#ifndef SSTRING_H_INCLUDED
#define SSTRING_H_INCLUDED
#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace sstring {

class cchars;

class String {
    private:
        unsigned id;
        void bind(unsigned id);
        friend class cchars;

        friend class boost::serialization::access;
        template <class Archive>
            void load(Archive &ar, const unsigned int version);
        template <class Archive>
            void save(Archive &ar, const unsigned int version) const;
        BOOST_SERIALIZATION_SPLIT_MEMBER();

        inline void refup();
        inline void refdn();
    public:
        String();
        String(const char * const chars);
        String(const String &sstr);
        String(const std::string& strng);
        String(unsigned id);
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
};

void load(const char * filename);
void save(const char * filename);
void prune();
void dump();
}
#endif
