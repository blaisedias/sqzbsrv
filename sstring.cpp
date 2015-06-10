#include <iostream>
#include <string.h>
#include <map>
#include <assert.h>
#include "sstring.h"

// for serialisation
#include <sstream>
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace sstring {

const bool debug_cchars=false;
const bool debug=false;
std::ostream& cout = std::cout;
#define nl std::endl
const char *sp = " ";

//static void *backstop=0;

static unsigned acquire_cchars(const char * cc);
static void read_ccmap_entry(std::ifstream& ifs);

//REGISTRY
struct ccompare : public std::binary_function<const char*, const char *, bool>
{
    bool operator()(const char *lhs, const char *rhs)
    {
        return strcmp(lhs,rhs) < 0;
    }
};

// string id to cchars instance map
typedef std::map<unsigned, const cchars*> IDCCMAP;
static IDCCMAP id_cc_map;
// null terminated char * to id map, char * is pointing to cchar instance chars member
typedef std::map<const char *, unsigned, ccompare> CCMAP;
static CCMAP cc_map;
// current ID value, * MUST * be serialized to maintain ID uniqueness.
// TODO: handle id overflow.
static unsigned cc_id = 100;


static char* new_cstr(const char * chars_in="", char* prev_chars=NULL)
{
    char * chars = new char[strlen(chars_in) + 1];
    strcpy(chars, chars_in);
    if(prev_chars)
        delete prev_chars;
    return chars;
}

class cchars
{
    private:
        friend class StaticInitializer;
        friend class String;
        friend unsigned acquire_cchars(const char * chars);
        friend void prune();
        friend void dump();
        friend void read_ccmap_entry(std::ifstream& ifs);
        friend void save(const char * filename);

        int ref_count;
        unsigned id;
        char * chars;

        // Prevent trivial assigment, ids and refcount need to be migrate
        cchars& operator=(const cchars&);
        cchars(const char * const chars_in):ref_count(0),id(0),chars(new_cstr(chars_in))
        {
            if (debug_cchars)
            {
                cout << this << " cchars(" << chars_in  << ")" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }

        cchars(const char * const chars_in, unsigned new_id):ref_count(0),id(new_id),chars(new_cstr(chars_in))
        {
            if (debug_cchars)
            {
                cout << this << " cchars(" << chars_in  << ", " << id << ")" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }

    // bleh! { for now constructors need to be public for boost serialization of containers of 
    public:
        // CTOR required for STL containers
        cchars():ref_count(0),id(0),chars(new_cstr())
        {
            if (debug_cchars)
                cout << this << " cchars()" << nl;
        }

        // copy constructor CTOR required for STL containers
        // Note move semantics on chars * will not work because of the way search_cc.chars has been setup.
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
            if (cs.id)
            {
                if (id_cc_map.count(cs.id))
                {
                    if (id_cc_map[cs.id] != &cs)
                        throw std::logic_error("sstring::cchars copy constructor: registry corrupted, id record does not match source cchars object");
                }
                id_cc_map[cs.id] = this;
            }
            ref_count = cs.ref_count;
            id = cs.id;
            // take ownership of the id.
            const_cast<cchars &>(cs).id = 0;
            chars = new_cstr(cs.chars, chars);
            if (debug_cchars)
            {
                cout << this << "           " << id << ", " << ref_count << " chars*=" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
        }

        // DTOR
        ~cchars()
        {
            if(debug_cchars)
            {
                cout << this << " ~cchars() ref_count=" << ref_count << " id=" << id << " chars*=" << (void *)chars << nl;
                cout << this << "           " << chars << nl;
            }
            if(id)
            {
//                id_cc_map[id] = (cchars *)backstop;
//                id_cc_map[id] = 0;
                id_cc_map.erase(id);
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

        // boost serialization support.
        template <class Archive>
        void save(Archive &ar,  const unsigned int version) const
        {
            // Store the unique id and the c string associated with that id
            ar & id;
            std::string tmp(chars);
            ar & tmp;
            if (debug_cchars)
            {
                std::cerr << "############################# " << std::setw(9) << std::setfill('0') << id << std::setw(0) << " " << tmp << std::endl;
            }
        }

        template <class Archive>
        void load(Archive &ar,  const unsigned int version)
        {
            std::string tmp;
            // FIXME: meaningful? if ref_count != 0 raise exception
            ref_count = 0;

            ar & id;
            ar & tmp;

            chars = new_cstr(tmp.c_str(), chars);

            // Loading into an existing id slot is undefined, abort.
            if (id_cc_map.count(id))
                throw std::logic_error("sstring::cchars.load id is already in use");
            // For now link this instance to the id.
            id_cc_map[id] = this;
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER();

        friend std::ostream& operator<< (std::ostream&, const cchars&);
};

std::ostream& operator<< (std::ostream& os,const cchars& cs)
{
    os << cs.chars;
    return os;
}

static unsigned acquire_cchars(const char * chars)
{
    CCMAP::iterator find = cc_map.find(chars);
    if (find == cc_map.end())
    {
        unsigned new_id = cc_id++;
        cchars * new_pcc= new cchars(chars, new_id);
        cc_map[new_pcc->chars] = new_id;
        id_cc_map[new_id] = new_pcc;
        if (debug)
            std::cout << "NEW   id=" << new_id << " " << cc_map.find(chars)->first << nl;
        return new_id;
    }
    else
    {
        if (debug)
            std::cout << "FOUND id=" << find->second << " " << find->first << nl;
    }
    return find->second;
}


void prune()
{
    for(IDCCMAP::iterator itr=id_cc_map.begin();
            itr != id_cc_map.end(); ++itr)
    {
        const cchars * pcchars = itr->second;
        if ((pcchars != NULL) && (pcchars->ref_count == 0))
        {
            if (debug_cchars)
                std::cerr << "Erasing  " << pcchars->id << " "<< *pcchars << nl;
            cc_map.erase(itr->second->chars);
            //FIXME: can we do id_cc_map.erase(itr->first) safely here?
            id_cc_map[itr->first] = NULL;
        }
    }
}

void dump()
{
    std::cout << "===================" << nl;
    std::cout << "ID to CCHARS" << nl;
    for(IDCCMAP::iterator itr=id_cc_map.begin();
            itr != id_cc_map.end(); itr++)
    {
        const cchars * pcchars = itr->second;
        if (pcchars)
            std::cout << "key=" << itr->first << " = id:" << pcchars->id << " rc: " << pcchars->ref_count << ", " << *pcchars << nl;
        else
            std::cout << "key=" << itr->first << " = " << pcchars << nl;
    }
    std::cout << "----------" << nl;
    std::cout << "CSTR to ID" << nl;
    for(CCMAP::iterator itr=cc_map.begin();
            itr != cc_map.end(); ++itr)
    {
        std::cout << "key=" << itr->first << " = " << itr->second << nl;
    }
    std::cout << "===================" << nl;
}

class StaticInitializer {
    public:
        StaticInitializer ()
        {
            // Ensure instantiation of the serialization methods  for String
            {
                std::vector<String> dummy;
                std::stringstream ss;
                boost::archive::text_oarchive oar(ss);
                oar & dummy;

                boost::archive::text_iarchive iar(ss);
                iar & dummy;

                id_cc_map[0] = new cchars("!!!default chars!!!");
                //backstop = (void *)id_cc_map[0];
                //FIXME:!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //cc_map[id_cc_map[0]] = 0;
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
    bind(acquire_cchars(chars));
    if (debug)
    {
        const cchars *pcc = id_cc_map[id];
        cout << this << " String(const chars *) cs=" << pcc  << " ref_count" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
        cout << "   +++ " << pcc->chars <<  nl;
    }
}

String::String(const std::string& strng): id(0)
{
    bind(acquire_cchars(strng.c_str()));
    if (debug)
    {
        const cchars *pcc = id_cc_map[id];
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
            const cchars *pcc = id_cc_map[id];
            cout << this << " String(CopyConstuctor) cs=" << pcc  << " ref_count=" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
            cout << "   +++ " << pcc->chars <<  nl;
        }
        else
            cout << this << " String(CopyConstuctor)  id=" << id <<  nl;

    }
}

String::String(unsigned new_id): id(0)
{
    if(new_id && (id_cc_map.count(new_id)==0))
    {
        // bind to unknown string.
        throw std::logic_error("sstring::String::String(unsigned id) id is not present in the registry");
    }
    id=new_id;
    refup();
    if (debug)
    {
        const cchars *pcc = id_cc_map[id];
        cout << this << " String(id) cs=" << pcc  << " ref_count=" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
        cout << "   +++ " << pcc->chars <<  nl;
    }
}

void String::refup()
{
    cchars *pcc = const_cast<cchars *>(id_cc_map[id]);
    pcc->ref_count++;
}

void String::refdn()
{
    cchars *pcc = const_cast<cchars *>(id_cc_map[id]);
//    if (pcc == backstop)
//        return;
    if (pcc == 0)
    {
        std::cerr << "@refdn for id=" << id << ", pcc == 0" << std::endl;
        return;
    }
    --(pcc->ref_count);
}

void String::bind(unsigned new_id)
{
    if (debug)
        cout << this << " bind(" << ")" << new_id << nl;

    // assign to self
    if (new_id == id)
        return;

    if(new_id && (id_cc_map.count(new_id)==0))
    {
        // bind to unknown string.
        std::cerr << " bind to " << new_id << " failed" << std::endl;
        throw std::logic_error("sstring::String::bind(unsigned id) id is not present in the registry");
    }

    if(id)
    {
        if (debug)
        {
            const cchars *pcc = id_cc_map[id];
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
            const cchars *pcc = id_cc_map[id];
            cout << this << " ~String(cs " << pcc  << ") ref_count=" << pcc->ref_count << " id=" << id << ", " << pcc->id <<  nl;
            cout << "   --- " << pcc->chars <<  nl;
        }
        refdn();
    }
    else
    {
        const cchars *pcc = id_cc_map[id];
        if (debug)
            cout << this << " ~String(cs " << pcc << ") id=" << id <<  nl;
    }
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
    const cchars *pcc = id_cc_map[id];
    return *pcc == cpp_str.c_str();
}

bool String::operator==(const char * c_str) const
{
    const cchars *pcc = id_cc_map[id];
    return *pcc == c_str;
}

bool String::operator<(const String& sstr) const
{
    cchars *pcc = const_cast<cchars *>(id_cc_map[id]);
    cchars *pcc_other = const_cast<cchars *>(id_cc_map[sstr.id]);
    return *pcc < *pcc_other;
}

bool String::operator<(const char *c_str) const
{
    cchars *pcc = const_cast<cchars *>(id_cc_map[id]);
    return *pcc < c_str;
}

std::string String::std_str() const
{
    const cchars *pcc = id_cc_map[id];
    return std::string(pcc->chars);
}

std::ostream& operator<< (std::ostream& os, const String& sstr)
{
    const cchars *pcc = id_cc_map[sstr.id];
    os << pcc->chars;
    return os;
}

const char * String::c_str() const
{
    const cchars *pcc = id_cc_map[id];
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
static void read_ccmap_entry(std::ifstream& ifs)
{
    char tmp;
    unsigned id;
    unsigned slen;

    ifs >> id;
    ifs >> slen;

    char chars[slen+1];
    ifs.get(tmp);
    if (slen)
        ifs.read(chars, slen+1);
    chars[slen] = 0;
    cchars *pcc = new cchars(chars, id);
    id_cc_map[id] = pcc;
    cc_map[pcc->chars] = id;
//    cout << id << "      '" << chars << "' " << pcc << std::endl;
}

void load(const char * filename)
{
    std::ifstream ifs(filename);
    if (ifs.is_open())
    {
        ifs >> cc_id;
        unsigned long cc_map_size;
        ifs >> cc_map_size;
        while(cc_map_size--)
        {
            read_ccmap_entry(ifs);
        }
    }
}

#if 0
void save(const char * filename)
{
    std::ofstream ofs(filename);
    if (ofs.is_open())
    {
        ofs << cc_id << '\n';
        unsigned long cc_map_size = cc_map.size();
        ofs << cc_map_size << '\n';
        for(CCMAP::iterator itr=cc_map.begin();
                itr != cc_map.end(); ++itr)
        {
            {
                unsigned long slen = strlen(itr->first);
                ofs << itr->second << " " << slen << " ";
                ofs << itr->first;
                ofs << '\n';
            }
        }
    }
}
#else
void save(const char * filename)
{
    std::ofstream ofs(filename);
    if (ofs.is_open())
    {
        ofs << cc_id << '\n';
        unsigned long cc_map_size = cc_map.size();
        ofs << cc_map_size << '\n';
        for(IDCCMAP::iterator itr=id_cc_map.begin();
                itr != id_cc_map.end(); ++itr)
        {
            if(itr->second)
            {
                unsigned long slen = strlen(itr->second->chars);
                ofs << itr->first << " " << slen << " ";
                ofs << itr->second->chars;
                ofs << '\n';
            }
        }
    }
}
#endif

} // namespace

