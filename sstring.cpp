#include <iostream>
#include <string.h>
#include <unordered_map>
#include <stack>
#include <assert.h>
#include "sstring.h"

// for serialisation
#include <sstream>
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace sstring {

const bool debug_cchars=false;
const bool debug=false;
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

//REGISTRY
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


class cchars;

static char* new_cstr(const char * chars_in=NULL, char* prev_chars=NULL)
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

class cchars
{
    private:
        unsigned id=0;
        char * chars;
        std::size_t hashv=0;
        int ref_count=0;

        friend class String;

        // Prevent trivial assignment.
        cchars& operator=(const cchars&);
    public:
        cchars(const char * const chars_in=NULL):chars(new_cstr(chars_in))
        {
            if (debug_cchars)
            {
                cout << this << " cchars(" << chars_in  << ")" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }

        cchars(unsigned new_id, const char * const chars_in, std::size_t hashval=0):id(new_id),chars(new_cstr(chars_in)),hashv(hashval)
        {
            if (debug_cchars)
            {
                cout << this << " cchars(" << chars_in  << ", " << id << ")" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }

#if 0
        // CTOR required for STL containers
        cchars():chars(new_cstr())
        {
            if (debug_cchars)
                cout << this << " cchars()" << nl;
        }

        // copy constructor CTOR required for STL containers
        cchars(const cchars& cs):ref_count(0),id(0),chars(new_cstr())
        {
            if (debug_cchars)
                cout << this << " cchars( CopyConstructor " << &cs  << ")" << id << ", " << ref_count << " chars*=" << (void *)chars << nl;
            if (cs.ref_count)
            {
                //Should never happen
                cout << " rc " << cs.ref_count << nl;
                throw std::logic_error("sstring::cchars copy constructor: copy from cchars object which has non 0 reference count");
            }
            ref_count = 0;
            id = cs.id;
            chars = new_cstr(cs.chars, chars);
            if (debug_cchars)
            {
                cout << this << "           " << id << ", " << ref_count << " chars*=" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }
#endif
        // DTOR
        ~cchars()
        {
            if(debug_cchars)
            {
                cout << this << " ~cchars() ref_count=" << ref_count << " id=" << id << " chars*=" << (void *)chars << nl;
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

        bool operator<(const cchars &cmp_cchars) const
        {
            return strcmp(chars, cmp_cchars.chars) < 0;
        }

        bool operator<(const cchars *cmp_cchars) const
        {
            return strcmp(chars, cmp_cchars->chars) < 0;
        }

        friend std::ostream& operator<< (std::ostream& os, const String& sstr);
        friend std::ostream& operator<< (std::ostream&, const cchars&);

        inline std::size_t hashvalue()
        {
            if (hashv == 0)
                hashv = std::hash<std::string>()(std::string(chars));
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

            chars = new_cstr(tmp.c_str(), chars);
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

std::ostream& operator<< (std::ostream& os, const cchars& cs)
{
    os << cs.chars;
    return os;
}

// string id to cchars instance map
typedef std::unordered_map<unsigned, const cchars*> IDCCMAP;
// null terminated char * to id map, char * is pointing to cchar instance chars member
//typedef std::map<const char *, unsigned, ccompare> CCMAP;
typedef std::unordered_map<const char *, unsigned, cc_hash, cc_equal_to > CCMAP;

class RegistryImpl : public Registry
{
    private:
        unsigned    currentID = 100;
    public:
        IDCCMAP id_pcc;
        CCMAP cc_id;

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
                    cchars *pcc = new cchars();
                    pcc->load(ifs);
                    id_pcc[pcc->v_id()] = pcc;
                    cc_id[pcc->v_str()] = pcc->v_id();
                }
            }
        }

        void save(const char * filename)
        {
            std::ofstream ofs(filename);
            if (ofs.is_open())
            {
                unsigned long cc_map_size = id_pcc.size();
                std::vector<unsigned> v;
                for(IDCCMAP::iterator itr=id_pcc.begin();
                        itr != id_pcc.end(); ++itr)
                    v.push_back(itr->first);
                std::sort(v.begin(), v.end());
        
                ofs << currentID << '\n';
                ofs << cc_map_size << '\n';

#if 0        
                for(IDCCMAP::iterator itr=id_pcc.begin();
                        itr != id_pcc.end(); ++itr)
                {
                    if(itr->second)
                    {
                        unsigned long slen = strlen(itr->second->chars);
                        ofs << itr->first << " " << slen << " " << itr->second->chars << '\n';
                    }
                }
#else
                for(auto id: v)
                {
                    {
                        id_pcc[id]->save(ofs);
                    }
                }
#endif
            }
        }

        void prune()
        {
            std::stack<unsigned> to_delete;
            for(IDCCMAP::iterator itr=id_pcc.begin();
                    itr != id_pcc.end(); ++itr)
            {
                const cchars * pcchars = itr->second;
                if ((pcchars != NULL) && (pcchars->v_ref_count() == 0))
                {
                    if (debug_cchars)
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
            std::cout << "ID to CCHARS" << nl;
            for(IDCCMAP::iterator itr=id_pcc.begin();
                    itr != id_pcc.end(); itr++)
            {
                const cchars * pcchars = itr->second;
                if (pcchars)
                    std::cout << "key=" << itr->first << " = id:" << pcchars->v_id() << " rc: " << pcchars->v_ref_count() << ", " << *pcchars << nl;
                else
                    std::cout << "key=" << itr->first << " = " << pcchars << nl;
            }
            std::cout << "----------" << nl;
            std::cout << "CSTR to ID" << nl;
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
                cchars * new_pcc= new cchars(new_id, chars);
                cc_id[new_pcc->v_str()] = new_id;
                id_pcc[new_id] = new_pcc;
                if (debug)
                    std::cout << "NEW   id=" << new_id << " " << cc_id.find(chars)->first << nl;
                return new_id;
            }
            else
            {
                if (debug)
                    std::cout << "FOUND id=" << find->second << " " << find->first << nl;
            }
            return find->second;
        }

        inline const cchars* getcc(unsigned id)
        {
            return id_pcc[id];
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
String::String(): id(0)
{
}

String::String(const char * const chars): id(0)
{
    bind(defaultRegister.acquire_cchars(chars));
    if (debug)
    {
        const cchars *pcc = defaultRegister.id_pcc[id];
        cout << this << " String(const chars *) cs=" << pcc  << " ref_count" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
        cout << "   +++ " << pcc->chars <<  nl;
    }
}

String::String(const std::string& strng): id(0)
{
    bind(defaultRegister.acquire_cchars(strng.c_str()));
    if (debug)
    {
        const cchars *pcc = defaultRegister.id_pcc[id];
        cout << this << " String(std::string) cs=" << pcc  << " ref_count=" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
        cout << "   +++ " << pcc->chars <<  nl;
    }
}

String::String(const String &sstr): id(0)
{
    bind(sstr.id);
    if (debug)
    {
        if(id)
        {
            const cchars *pcc = defaultRegister.id_pcc[id];
            cout << this << " String(CopyConstuctor) cs=" << pcc  << " ref_count=" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
            cout << "   +++ " << pcc->chars <<  nl;
        }
        else
            cout << this << " String(CopyConstuctor)  id=" << id <<  nl;

    }
}

String::String(unsigned new_id): id(0)
{
    if(new_id && (defaultRegister.id_pcc.count(new_id)==0))
    {
        // bind to unknown string.
        throw std::logic_error("sstring::String::String(unsigned id) id is not present in the registry");
    }
    id=new_id;
    refup();
    if (debug)
    {
        const cchars *pcc = defaultRegister.id_pcc[id];
        cout << this << " String(id) cs=" << pcc  << " ref_count=" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
        cout << "   +++ " << pcc->chars <<  nl;
    }
}

void String::refup()
{
    cchars *pcc = const_cast<cchars *>(defaultRegister.id_pcc[id]);
    pcc->ref_count++;
}

void String::refdn()
{
    IDCCMAP::iterator find = defaultRegister.id_pcc.find(id);
    if (find != defaultRegister.id_pcc.end())
    {
        cchars *pcc = const_cast<cchars *>(find->second);
        if (pcc == 0)
        {
            std::cerr << "@refdn for id=" << id << ", pcc == 0" << std::endl;
            return;
        }
        --(pcc->ref_count);
    }
    else
    {
//        std::cerr << "@refdn for absent id=" << id << std::endl;
    }
}

void String::bind(unsigned new_id)
{
    if (debug)
        cout << this << " bind(" << ")" << new_id << nl;

    // assign to self
    if (new_id == id)
        return;

    if(new_id && (defaultRegister.id_pcc.count(new_id)==0))
    {
        // bind to unknown string.
        std::cerr << " bind to " << new_id << " failed" << std::endl;
        throw std::logic_error("sstring::String::bind(unsigned id) id is not present in the registry");
    }

    if(id)
    {
        if (debug)
        {
            const cchars *pcc = defaultRegister.id_pcc[id];
            cout << "unbind   --- " << pcc->chars <<  nl;
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
        if (debug)
        {
            const cchars *pcc = defaultRegister.id_pcc[id];
            cout << this << " ~String(cs " << pcc  << ") ref_count=" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
            cout << "   --- " << pcc->chars <<  nl;
        }
        refdn();
    }
    else
    {
        const cchars *pcc = defaultRegister.id_pcc[id];
        if (debug)
            cout << this << " ~String(cs " << pcc << ") id=" << id <<  nl;
    }
}

std::size_t String::hash() const
{
//    return oat_hash(defaultRegister.id_pcc[id]->chars);
    return const_cast<cchars *>(defaultRegister.id_pcc[id])->hashvalue();
}

String& String::operator=(const String& sstr)
{
    bind(sstr.id);

    if (debug)
        cout << this << " =  " << &sstr <<  nl;
    return *this;
}

bool String::operator==(const String& sstr) const
{
    return id == sstr.id;
}

bool String::operator==(const std::string& cpp_str) const
{
    const cchars *pcc = defaultRegister.id_pcc[id];
    return *pcc == cpp_str.c_str();
}

bool String::operator==(const char * c_str) const
{
    const cchars *pcc = defaultRegister.id_pcc[id];
    return *pcc == c_str;
}

bool String::operator<(const String& sstr) const
{
    cchars *pcc = const_cast<cchars *>(defaultRegister.id_pcc[id]);
    cchars *pcc_other = const_cast<cchars *>(defaultRegister.id_pcc[sstr.id]);
    return *pcc < *pcc_other;
}

bool String::operator<(const char *c_str) const
{
    cchars *pcc = const_cast<cchars *>(defaultRegister.id_pcc[id]);
    return *pcc < c_str;
}

std::string String::std_str() const
{
    const cchars *pcc = defaultRegister.id_pcc[id];
    return std::string(pcc->chars);
}

std::ostream& operator<< (std::ostream& os, const String& sstr)
{
    const cchars *pcc = defaultRegister.id_pcc[sstr.id];
    os << pcc->chars;
    return os;
}

const char * String::c_str() const
{
    const cchars *pcc = defaultRegister.id_pcc[id];
    return pcc->chars;
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
    if (debug)
        cout << this << " save(" << ")" << id << nl;
    ar & id;
}

// Module functions

} // namespace

