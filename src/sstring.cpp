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
#include <thread>
#include <assert.h>
#include "sstring.h"

// for serialisation
#include <sstream>
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace sstring {

const bool debug_ref_counts=false;
const bool debug_prune=true;
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
        unsigned dom=0;
        unsigned id=0;
        char * chars;
        std::size_t hashv=0;
        bool hashv_set = false;
        std::atomic<int> ref_count{0};

        friend class RegistryImpl;

        // Non copyable
        rc_cstr& operator=(const rc_cstr&) = delete;
        rc_cstr& operator=(rc_cstr&&) = delete;

        //Non movable
        rc_cstr(rc_cstr const&) = delete;
        rc_cstr(rc_cstr&&) = delete;
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
        void save(std::ostream& ofs) const
        {
            unsigned slen = strlen(chars);
//            ofs << id << " " << hashv << " " << slen << sep << chars << '\n';
            ofs << id << " " << " " << slen << sep << chars << '\n';
        }

        void load(std::istream& ifs)
        {
            char mysep;
            unsigned slen;
            ifs >> id;
//            ifs >> hashv;
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

class Domain: public SerializationContext {
    private:
        // Non copyable
        Domain& operator=(const Domain&) = delete;
        Domain& operator=(Domain&&) = delete;

        //Non movable
        Domain(Domain const&) = delete;
        Domain(Domain&&) = delete;

        unsigned mask;
        std::unordered_map<unsigned, unsigned> id_remap;
        friend class RegistryImpl;
    public:
        Domain(unsigned mask)
        {
            this->mask = mask;
        }

        ~Domain()
        {
        }

        inline bool is_remapped()
        {
            return id_remap.size() != 0;
        }

        unsigned remapped_id(unsigned id_in)
        {
            unsigned id_out = id_in;
            if (id_remap.size())
            {
                std::unordered_map<unsigned, unsigned>::iterator find =
                    id_remap.find(id_in);
                if (find != id_remap.end())
                    id_out = find->second;
            }
            return id_out;
        }
};

#ifdef  SSTRING_HAS_DEFAULT_CONTEXT
static Domain* defaultDomain = new Domain(1);
#else
static Domain* defaultDomain = NULL;
#endif
static thread_local Domain* currentDomain = defaultDomain;

// string id to rc_cstr instance map
typedef std::unordered_map<unsigned, rc_cstr*> IDCCMAP;
// null terminated char * to id map, char * is pointing to cchar instance chars member
typedef std::unordered_map<const char *, unsigned, cc_hash, cc_equal_to > CCMAP;

class RegistryImpl : public Registry
{
    private:
        unsigned    initialID = 100;
        std::atomic<unsigned> currentID{initialID};
        // String and ID registry, consists of two hash tables,
        // 1) ID to ref counted string object
        // 2) C-string to ID
        IDCCMAP id_pcc;
        CCMAP cc_id;
        std::mutex mtx;

        std::mutex domain_factory_mutex; 
        unsigned assigned_doms = 0;
        std::unordered_map<class Domain*, class Domain*> known_domains;

        friend ContextGuard;
        Domain *domainFromSerialisationContext(SerializationContext* psc)
        {
            try
            {
                return known_domains.at(dynamic_cast <Domain*>(psc));
            }
            catch (...)
            {
                throw std::invalid_argument("Invalid serialization context type");
            }
        }

        Domain* getDomain()
        {
            std::lock_guard<std::mutex> lockg(domain_factory_mutex);

            unsigned mask = 0x1;
            while(mask & (mask & assigned_doms))
                mask <<= 1;

            if(mask)
            {
                Domain* dom = new Domain(mask);
                assigned_doms |= mask;
                known_domains[dom] = dom;
                return dom;
            }
            throw std::overflow_error("SerializationContext");
        }

        void freeDomain(Domain* dom)
        {
            std::lock_guard<std::mutex> lockg(domain_factory_mutex);

            if (defaultDomain && dom == defaultDomain)
                throw std::invalid_argument("Invalid serialization context, default context cannot be freed");

            try
            {
                dom = known_domains.at(dom);
            }
            catch (...)
            {
                throw std::invalid_argument("Invalid serialization context type");
            }

            if (assigned_doms & dom->mask)
            {
                assigned_doms ^= dom->mask;
                known_domains.erase(dom);
                delete dom;
            }
        }

    public:
        ~RegistryImpl() {}
        RegistryImpl()
        {
            if (defaultDomain)
            {
                known_domains[defaultDomain] = defaultDomain;
                assigned_doms |= 1;
            }
        }

        void load(std::istream& ifs)
        {
            Domain *domain = currentDomain;
            if (NULL == domain)
                throw std::logic_error("sstring::RegistryImpl::load invalid context thread local");
            {
                unsigned ceilingID;
                ifs >> ceilingID;
                bool merge = !currentID.compare_exchange_strong(initialID, ceilingID);
                unsigned long cc_map_size;
                ifs >> cc_map_size;
                if (merge)
                {
                    // ensure that currentID >= ceilingID.
                    // This improves the prospects that IDs loaded during this
                    // serialisation will be available for assignment.
                    // We cannot just test if currentID < ceilingID and then
                    // set ceilingID into currentID, we risk reducing the value
                    // of currentID if it gets updated concurrently to a 
                    // value > ceilingID between the test and the store.
                    unsigned cid;
                    while((cid = currentID.load()) < ceilingID)
                    {
                        currentID.compare_exchange_strong(cid, ceilingID);
                    }
                }
                while(cc_map_size--)
                {
                    rc_cstr *pcc = new rc_cstr();
                    pcc->load(ifs);
                    if (merge)
                    {
                        // Merge:
                        CCMAP::iterator find_ccid = cc_id.find(pcc->chars);
                        if (find_ccid != cc_id.end())
                        {
                            // String already exists.
                            rc_cstr *curr_pcc = id_pcc[find_ccid->second];
                            curr_pcc->dom |= domain->mask;
                            if (pcc->id != curr_pcc->id)
                            {
                                // domain id_map is unordered_map<unsigned, unsigned>
                                // Domain id map is used by clients when deserializing,
                                // to convert serialized id values to live values.
                                domain->id_remap[pcc->v_id()] = curr_pcc->v_id();
                            }

                            delete(pcc);
                            continue;
                        }

                        // @here, currentID must be greater than an loaded ID
                        // since we've ensured that the its value is >= to the
                        // ceilingID value associated with this serialisation
                        // so if the ID value is in use, we have to map the
                        // loaded ID to a newly reserved ID value.
                        IDCCMAP::iterator find_idcc = id_pcc.find(pcc->id);
                        if (find_idcc != id_pcc.end())
                        {
                            // ID already in use
                            unsigned newid = currentID++;
                            domain->id_remap[pcc->id] = newid;
                            pcc->id = newid;
                        }
                    }
                    pcc->dom |= domain->mask;
                    id_pcc[pcc->v_id()] = pcc;
                    cc_id[pcc->v_str()] = pcc->v_id();
                }
            }
        }

        // If pruning is required it should be done before saving.
        void save(std::ostream& ofs)
        {
            Domain *domain = currentDomain;
            if (NULL == domain)
                throw std::logic_error("sstring::RegistryImpl::load invalid context thread local");
            {
                unsigned ceilingID = 0;            
//                unsigned long cc_map_size = id_pcc.size();
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
                        if ((itr->second->dom & domain->mask) == 0)
                            continue;

                        v.push_back(itr->first);
                        refup(itr->first);
                        if(itr->second->v_id() > ceilingID)
                            ceilingID = itr->second->v_id();
                    }
                }
                std::sort(v.begin(), v.end());
                ++ceilingID;
        
                ofs << ceilingID << '\n';
                ofs << v.size() << '\n';

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
                    if (debug_rc_cstr || debug_prune)
                        std::cerr << "@prune: Erasing  " << pcchars->v_id() << " "<< *pcchars << nl;
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

        inline bool exists(const char* chars)
        {
            CCMAP::iterator find = cc_id.find(chars);
            return find != cc_id.end();
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
            if (debug_ref_counts)
            {
                if(id_pcc[id]->ref_count == 0)
                    std::cerr << "@refup for id=" << id << ", ref_count == 0," << id_pcc[id]->chars << std::endl;
            }
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
                if (debug_ref_counts)
                {
                    if (pcc->ref_count == 0)
                        std::cerr << "@refdn for id=" << id << ", ref_count == 0," << pcc->chars << std::endl;
                }
                if (pcc->ref_count == 0 && Registry::delete_immediately)
                {
                    if (debug_rc_cstr || debug_prune)
                        std::cerr << "@refdn: Erasing  " << pcc->v_id() << " "<< pcc->v_str() << nl;
                    cc_id.erase(pcc->chars);
                    id_pcc.erase(pcc->id);
                    delete pcc;
                }
            }
            else
            {
//                std::cerr << "@refdn for absent id=" << id << std::endl;
            }
        }

        inline void setSerializationContext(unsigned id, SerializationContext* psc)
        {
            if (psc)
            {
                //if(typeid(defaultDomain) == typeid(psc))
                //    throw std::invalid_argument("Invalid serialization context type");
                //id_pcc[id]->dom |= (dynamic_cast <Domain*>(psc))->mask;
                id_pcc[id]->dom |= domainFromSerialisationContext(psc)->mask;
            }
        }

        SerializationContext* makeSerializationContext()
        {
            Domain *dom = getDomain();
            return dynamic_cast<SerializationContext*>(dom);
        }

//        inline SerializationContext* getDefaultSerializationContext()
//        {
//            assert(defaultDomain != NULL);
//            return dynamic_cast<SerializationContext*>(defaultDomain);
//        }

        void setDeleteImmediately(bool new_value)
        {
            if (new_value != Registry::delete_immediately)
            {
                Registry::delete_immediately = new_value;
                if (delete_immediately)
                    prune();
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
String::String(const char * const chars, SerializationContext *psc): id(0)
{
    if (NULL == psc)
        throw std::logic_error("sstring::RegistryImpl::load invalid context thread local");
    bind(defaultRegister.acquire_cchars(chars));
    setSerializationContext(psc);
    if (debug_string)
    {
        const rc_cstr *pcc = defaultRegister.getcc(id);
        cout << this << " String(const chars *) cs=" << pcc  << " ref_count" << pcc->v_ref_count() << " id=" << id << ", " << pcc->v_id() <<  nl;
        cout << "   +++ " << pcc->v_str() <<  nl;
    }
}

String::String(const char * const chars):String(chars, currentDomain)
{
}

String::String(const std::string& strng, SerializationContext *psc): id(0)
{
    if (NULL == psc)
        throw std::logic_error("sstring::RegistryImpl::load invalid context thread local");
    bind(defaultRegister.acquire_cchars(strng.c_str()));
    setSerializationContext(psc);
    if (debug_string)
    {
        const rc_cstr *pcc = defaultRegister.getcc(id);
        cout << this << " String(std::string) cs=" << pcc  << " ref_count=" << pcc->v_ref_count() << " id=" << id << ", " << pcc->v_id() <<  nl;
        cout << "   +++ " << pcc->v_str() <<  nl;
    }
}

String::String(const std::string& strng):String(strng, currentDomain)
{
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
            std::cerr << "sstring::String::String(unsigned id) " << id << "is not present in the registry" << std::endl;
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
    unsigned ar_id;
    unsigned mar_id;
    ar & ar_id;
    mar_id = currentDomain->remapped_id(ar_id);
    bind(mar_id);
}

template <class Archive>
    void String::save(Archive &ar, const unsigned int version) const
{
    if (debug_string)
        cout << this << " save(" << ")" << id << nl;
    ar & id;
}

void String::setSerializationContext(SerializationContext *psc)
{
    defaultRegister.setSerializationContext(id, psc);
}

ContextGuard::ContextGuard(SerializationContext *psc)
{
    assert(psc != NULL);
//    std::cerr << "ContextGuard(" << psc << ") -> "<< currentDomain <<  std::endl;
    prev = currentDomain;
    currentDomain = defaultRegister.domainFromSerialisationContext(psc);
}

ContextGuard::~ContextGuard()
{
//    std::cerr << "~ContextGuard " << prev << "-> " << currentDomain << std::endl;
    if (prev != NULL)
        currentDomain = defaultRegister.domainFromSerialisationContext(prev);
    else
        currentDomain = NULL;
}

// Module functions

} // namespace

