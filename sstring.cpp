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
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <stack>
#include <atomic>
#include <mutex>
#include <assert.h>
#include "sstring.h"

// for serialisation
#include <sstream>
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace sstring {

const bool debug_rc_cstr=false;
const bool debug_registry=false;
const bool debug_string=false;
std::ostream& cout = std::cout;
#define nl std::endl
const char sep = ';';

std::size_t oat_hash(const char * cs)
{
    std::size_t h = 0;
    const unsigned char* ucs = (const unsigned char*)cs;

    if (ucs)
    {
        while(*ucs)
        {
            h += *ucs;
            h += (h << 10);
            h ^= (h >> 6);
            ++ucs;
        }

        h += (h << 3);
        h ^= (h >> 11);
        h += (h << 15);
    }
    return h;
}

struct ccompare : public std::binary_function<const char*, const char *, bool>
{
    bool operator()(const char *lhs, const char *rhs)
    {
        return strcmp(lhs,rhs) < 0;
    }
};

struct cc_equal_to : public std::binary_function<const char*, const char *, bool>
{
    bool operator()(const char *lhs, const char *rhs) const
    {
        if (lhs == rhs)
            return true;
        if ((lhs == 0) || (rhs == 0))
            return false; 
        return strcmp(lhs,rhs) == 0;
    }
};

struct cc_hash
{
    std::size_t operator()(const char *cs) const
    {
        return std::hash<std::string>()(std::string(cs));
//        return oat_hash(cs);        
    }
};


static char* new_chars(const char * chars_in=NULL, char* prev_chars=NULL)
{
    char* chars;
    if (chars_in)
    {
        chars = new char[strlen(chars_in) + 1];
        strcpy(chars, chars_in);
    }
    else
    {
        chars = new char[1];
        chars[0] = 0;
    }

    if(prev_chars)
        delete prev_chars;
    return chars;
}

class rc_cstr
{
    private:
        unsigned id=0;
        char * chars;
        std::size_t hashv=0;
        bool hashv_set = false;
        std::atomic<int> ref_count{0};

        friend class RegistryImpl;

        // Prevent trivial assignment.
        rc_cstr& operator=(const rc_cstr&);
    public:
        rc_cstr(const char * const chars_in=NULL):chars(new_chars(chars_in))
        {
            if (debug_rc_cstr)
            {
                cout << this << " rc_cstr(" << chars_in  << ")" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }

        rc_cstr(unsigned new_id, const char * const chars_in, std::size_t hashval=0):
            id(new_id), chars(new_chars(chars_in)), hashv(hashval), hashv_set(hashval != 0)
        {
            if (debug_rc_cstr)
            {
                cout << this << " rc_cstr(" << chars_in  << ", " << id << ")" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }

#if 0
        // CTOR required for STL containers
        rc_cstr():chars(new_chars())
        {
            if (debug_rc_cstr)
                cout << this << " rc_cstr()" << nl;
        }

        // copy constructor CTOR required for STL containers
        rc_cstr(const rc_cstr& cs):ref_count(0),id(0),chars(new_chars())
        {
            if (debug_rc_cstr)
                cout << this << " rc_cstr( CopyConstructor " << &cs  << ")" << id << ", " << ref_count << " chars*=" << (void *)chars << nl;
            if (cs.ref_count)
            {
                //Should never happen
                cout << " rc " << cs.ref_count << nl;
                throw std::logic_error("sstring::rc_cstr copy constructor: copy from rc_cstr object which has non 0 reference count");
            }
            ref_count = 0;
            id = cs.id;
            chars = new_chars(cs.chars, chars);
            if (debug_rc_cstr)
            {
                cout << this << "           " << id << ", " << ref_count << " chars*=" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }
#endif
        // DTOR
        ~rc_cstr()
        {
            if(debug_rc_cstr)
            {
                cout << this << " ~rc_cstr() ref_count=" << ref_count << " id=" << id << " chars*=" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
            delete [] chars;
        }

        bool operator==(const char *cmp_chars) const
        {
            return strcmp(chars, cmp_chars) == 0;
        }

        bool operator<(const char *cmp_chars) const
        {
            return strcmp(chars, cmp_chars) < 0;
        }

        bool operator<(const rc_cstr &cmp_cchars) const
        {
            // TODO: make this faster.
            return strcmp(chars, cmp_cchars.chars) < 0;
        }

        bool operator<(const rc_cstr *cmp_cchars) const
        {
            // TODO: make this faster.
            return strcmp(chars, cmp_cchars->chars) < 0;
        }

        bool operator>(const rc_cstr &cmp_cchars) const
        {
            // TODO: make this faster.
            return strcmp(chars, cmp_cchars.chars) > 0;
        }

        bool operator>(const rc_cstr *cmp_cchars) const
        {
            // TODO: make this faster.
            return strcmp(chars, cmp_cchars->chars) > 0;
        }

        friend std::ostream& operator<< (std::ostream& os, const String& sstr);
        friend std::ostream& operator<< (std::ostream&, const rc_cstr&);

        inline std::size_t hashvalue()
        {
            if (!hashv_set)
            {
                hashv = std::hash<std::string>()(std::string(chars));
                hashv_set = true;
            }
            return hashv;
        }

        inline const char* const v_str() const
        {
            return chars;
        }

        inline unsigned v_ref_count() const
        {
            return ref_count;
        }

        inline unsigned v_id() const
        {
            return id;
        }
#if 0
        // boost serialization support.
        template <class Archive>
        void save(Archive &ar,  const unsigned int version) const
        {
            std::string tmp(chars);
            // Store the unique id and the c string associated with that id
            ar & id;
            ar & hashv;
            ar & tmp;
        }

        template <class Archive>
        void load(Archive &ar,  const unsigned int version)
        {
            std::string tmp;
            // FIXME: meaningful? if ref_count != 0 raise exception
            ref_count = 0;

            ar & id;
            ar & hashv;
            ar & tmp;

            chars = new_chars(tmp.c_str(), chars);
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER();
#endif
        void save(std::ofstream& ofs) const
        {
            unsigned slen = strlen(chars);
            ofs << id << " " << hashv << " " << slen << sep << chars << '\n';
        }

        void load(std::ifstream& ifs)
        {
            char mysep;
            unsigned slen;
            ifs >> id;
            ifs >> hashv;
            ifs >> slen;
            ifs.get(mysep);
            chars = new char[slen + 1];
            if (slen)
                ifs.read(chars, slen + 1);
            chars[slen] = 0;
        }
};

std::ostream& operator<< (std::ostream& os, const rc_cstr& cs)
{
    os << cs.chars;
    return os;
}

// string id to rc_cstr instance map
typedef std::unordered_map<unsigned, rc_cstr*> IDCCMAP;
// null terminated char * to id map, char * is pointing to cchar instance chars member
typedef std::unordered_map<const char *, unsigned, cc_hash, cc_equal_to > CCMAP;

class RegistryImpl : public Registry
{
    private:
        unsigned    currentID = 100;
        // String and ID registry, consists of two hash tables,
        // 1) ID to ref counted string object
        // 2) C-string to ID
        IDCCMAP id_pcc;
        CCMAP cc_id;
        std::mutex mtx;

    public:
        ~RegistryImpl() {};
        RegistryImpl() {};

        void load(const char * filename)
        {
            std::ifstream ifs(filename);
            if (ifs.is_open())
            {
                ifs >> currentID;
                unsigned long cc_map_size;
                ifs >> cc_map_size;
                while(cc_map_size--)
                {
                    rc_cstr *pcc = new rc_cstr();
                    pcc->load(ifs);
                    id_pcc[pcc->v_id()] = pcc;
                    cc_id[pcc->v_str()] = pcc->v_id();
                }
            }
        }

        // If pruning is required it should be done before saving.
        void save(const char * filename)
        {
            std::ofstream ofs(filename);
            if (ofs.is_open())
            {
                unsigned long cc_map_size = id_pcc.size();
                std::vector<unsigned> v;
                // Serialise a snapshot:
                //  1) lock,
                //  2) record the ids we want to write out,
                //  3) increment the reference count for ids we want to write,
                //      to ensure that rc_cstr instances are kept alive till
                //      they've been written out.
                {
                    std::lock_guard<std::mutex> lockg(mtx);
                    v.reserve(id_pcc.size());
                    for(IDCCMAP::iterator itr=id_pcc.begin();
                            itr != id_pcc.end(); ++itr)
                    {
                        v.push_back(itr->first);
                        refup(itr->first);
                    }
                }
                std::sort(v.begin(), v.end());
        
                ofs << currentID << '\n';
                ofs << cc_map_size << '\n';

                for(auto id: v)
                {
                    id_pcc[id]->save(ofs);
                    refdn(id);
                }
            }
        }

        void prune()
        {
            std::stack<unsigned> to_delete;
            for(IDCCMAP::iterator itr=id_pcc.begin();
                    itr != id_pcc.end(); ++itr)
            {
                const rc_cstr * pcchars = itr->second;
                if ((pcchars != NULL) && (pcchars->v_ref_count() == 0))
                {
                    if (debug_rc_cstr)
                        std::cerr << "Erasing  " << pcchars->v_id() << " "<< *pcchars << nl;
                    to_delete.push(pcchars->v_id());
                }
            }

            while(!to_delete.empty())
            {
                {
                    cc_id.erase(id_pcc[to_delete.top()]->v_str());
                    delete id_pcc[to_delete.top()];
                    id_pcc.erase(to_delete.top());
                }
                to_delete.pop();
            }
        }

        void dump()
        {
            std::cout << "===================" << nl;
            std::cout << "ID to RC_CSTR" << nl;
            for(IDCCMAP::iterator itr=id_pcc.begin();
                    itr != id_pcc.end(); itr++)
            {
                const rc_cstr * pcchars = itr->second;
                if (pcchars)
                    std::cout << "key=" << itr->first << " = id:" << pcchars->v_id() << " rc: " << pcchars->v_ref_count() << ", " << *pcchars << nl;
                else
                    std::cout << "key=" << itr->first << " = " << pcchars << nl;
            }
            std::cout << "----------" << nl;
            std::cout << "RC_CSTR to ID" << nl;
            for(CCMAP::iterator itr=cc_id.begin();
                    itr != cc_id.end(); ++itr)
            {
                std::cout << "key=" << itr->first << " = " << itr->second << nl;
            }
            std::cout << "===================" << nl;
        }

        unsigned acquire_cchars(const char * chars)
        {
            CCMAP::iterator find = cc_id.find(chars);
            if (find == cc_id.end())
            {
                unsigned new_id = currentID++;
                rc_cstr * new_pcc= new rc_cstr(new_id, chars);
                cc_id[new_pcc->v_str()] = new_id;
                id_pcc[new_id] = new_pcc;
                if (debug_registry)
                    std::cout << "NEW   id=" << new_id << " " << cc_id.find(chars)->first << nl;
                return new_id;
            }
            else
            {
                if (debug_registry)
                    std::cout << "FOUND id=" << find->second << " " << find->first << nl;
            }
            return find->second;
        }

        inline const rc_cstr* getcc(unsigned id)
        {
            return id_pcc[id];
        }

        inline int getcc_count(unsigned id)
        {
            return id_pcc.count(id);
        } 

        inline void refup(unsigned id)
        {
            ++(id_pcc[id]->ref_count);
        }

        // dereferencing at program shutdown is disorderly,
        // and so this is more complicated :-(.
        inline void refdn(unsigned id)
        {
            IDCCMAP::iterator find = id_pcc.find(id);
            if (find != id_pcc.end())
            {
                rc_cstr *pcc = find->second;
                if (pcc == 0)
                {
                    std::cerr << "@refdn for id=" << id << ", pcc == 0" << std::endl;
                    return;
                }
                --(pcc->ref_count);
            }
            else
            {
//                std::cerr << "@refdn for absent id=" << id << std::endl;
            }
        }
};

static RegistryImpl defaultRegister;
Registry& getRegistry()
{
    return defaultRegister;
}

class StaticInitializer {
    public:
        StaticInitializer ()
        {
            // Ensure instantiation of template serialization methods for String
            {
                std::vector<String> dummy;
                std::stringstream ss;
                boost::archive::text_oarchive oar(ss);
                oar & dummy;

                boost::archive::text_iarchive iar(ss);
                iar & dummy;
            }
        }
};

static StaticInitializer static_init;

//== String functions
//
String::String(const char * const chars): id(0)
{
    bind(defaultRegister.acquire_cchars(chars));
    if (debug_string)
    {
        const rc_cstr *pcc = defaultRegister.getcc(id);
        cout << this << " String(const chars *) cs=" << pcc  << " ref_count" << pcc->v_ref_count() << " id=" << id << ", " << pcc->v_id() <<  nl;
        cout << "   +++ " << pcc->v_str() <<  nl;
    }
}

String::String(const std::string& strng): id(0)
{
    bind(defaultRegister.acquire_cchars(strng.c_str()));
    if (debug_string)
    {
        const rc_cstr *pcc = defaultRegister.getcc(id);
        cout << this << " String(std::string) cs=" << pcc  << " ref_count=" << pcc->v_ref_count() << " id=" << id << ", " << pcc->v_id() <<  nl;
        cout << "   +++ " << pcc->v_str() <<  nl;
    }
}

String::String(const String &sstr): id(0)
{
    bind(sstr.id);
    if (debug_string)
    {
        if(id)
        {
            const rc_cstr *pcc = defaultRegister.getcc(id);
            cout << this << " String(CopyConstuctor) cs=" << pcc  << " ref_count=" << pcc->v_ref_count() << " id=" << id << ", " << pcc->v_id() <<  nl;
            cout << "   +++ " << pcc->v_str() <<  nl;
        }
        else
            cout << this << " String(CopyConstuctor)  id=" << id <<  nl;

    }
}

String::String(unsigned new_id): id(0)
{
    if (new_id)
    {
        if(defaultRegister.getcc_count(new_id)==0)
        {
            // bind to unknown string.
            throw std::logic_error("sstring::String::String(unsigned id) id is not present in the registry");
        }
        id=new_id;
        refup();
        if (debug_string)
        {
            const rc_cstr *pcc = defaultRegister.getcc(id);
            cout << this << " String(id) cs=" << pcc  << " ref_count=" << pcc->v_ref_count() << " id=" << id << ", " << pcc->v_id() <<  nl;
            cout << "   +++ " << pcc->v_str() <<  nl;
        }
    }
}

void String::refup()
{
    defaultRegister.refup(id);
}

void String::refdn()
{
    defaultRegister.refdn(id);
}

void String::bind(unsigned new_id)
{
    if (debug_string)
        cout << this << " bind(" << ")" << new_id << nl;

    // assign to self
    if (new_id == id)
        return;

    if(new_id && (defaultRegister.getcc_count(new_id)==0))
    {
        // bind to unknown string.
        std::cerr << " bind to " << new_id << " failed" << std::endl;
        throw std::logic_error("sstring::String::bind(unsigned id) id is not present in the registry");
    }

    if(id)
    {
        if (debug_string)
        {
            const rc_cstr *pcc = defaultRegister.getcc(id);
            cout << "unbind   --- " << pcc->v_str() <<  nl;
        }
        
        refdn();
        id=0;
    }

    if (new_id == 0)
    {
        id = 0;
        return;
    }

    id=new_id;
    refup();
}

String::~String()
{
    if (id)
    {
        if (debug_string)
        {
            const rc_cstr *pcc = defaultRegister.getcc(id);
            cout << this << " ~String(cs " << pcc  << ") ref_count=" << pcc->v_ref_count() << " id=" << id << ", " << pcc->v_id() <<  nl;
            cout << "   --- " << pcc->v_str() <<  nl;
        }
        refdn();
    }
    else
    {
        const rc_cstr *pcc = defaultRegister.getcc(id);
        if (debug_string)
            cout << this << " ~String(cs " << pcc << ") id=" << id <<  nl;
    }
}

std::size_t String::hash() const
{
//    return oat_hash(defaultRegister.getcc(id)->chars);
    return const_cast<rc_cstr *>(defaultRegister.getcc(id))->hashvalue();
}

String& String::operator=(const String& sstr)
{
    bind(sstr.id);

    if (debug_string)
        cout << this << " =  " << &sstr <<  nl;
    return *this;
}

bool String::operator==(const String& sstr) const
{
    return id == sstr.id;
}

bool String::operator==(const std::string& cpp_str) const
{
    const rc_cstr *pcc = defaultRegister.getcc(id);
    return *pcc == cpp_str.c_str();
}

bool String::operator==(const char * c_str) const
{
    const rc_cstr *pcc = defaultRegister.getcc(id);
    return *pcc == c_str;
}

bool String::operator<(const String& sstr) const
{
    rc_cstr *pcc = const_cast<rc_cstr *>(defaultRegister.getcc(id));
    rc_cstr *pcc_other = const_cast<rc_cstr *>(defaultRegister.getcc(sstr.id));
    return *pcc < *pcc_other;
}

bool String::operator>(const String& sstr) const
{
    rc_cstr *pcc = const_cast<rc_cstr *>(defaultRegister.getcc(id));
    rc_cstr *pcc_other = const_cast<rc_cstr *>(defaultRegister.getcc(sstr.id));
    return *pcc > *pcc_other;
}

bool String::operator<(const char *c_str) const
{
    rc_cstr *pcc = const_cast<rc_cstr *>(defaultRegister.getcc(id));
    return *pcc < c_str;
}

std::string String::std_str() const
{
    const rc_cstr *pcc = defaultRegister.getcc(id);
    return std::string(pcc->v_str());
}

std::ostream& operator<< (std::ostream& os, const String& sstr)
{
    const rc_cstr *pcc = defaultRegister.getcc(sstr.id);
    os << pcc->v_str();
    return os;
}

const char * String::c_str() const
{
    const rc_cstr *pcc = defaultRegister.getcc(id);
    return pcc->v_str();
}

template <class Archive>
    void String::load(Archive &ar, const unsigned int version)
{
    unsigned id;
    ar & id;
    bind(id);
}

template <class Archive>
    void String::save(Archive &ar, const unsigned int version) const
{
    if (debug_string)
        cout << this << " save(" << ")" << id << nl;
    ar & id;
}

// Module functions

} // namespace

