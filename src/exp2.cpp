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
#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include "sstring.h"
#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << (x) ) ).str()
using namespace std;

// for serialisation
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

const bool cc_dump=false;

void sload(const char* filename, vector<sstring::String>& v, sstring::SerializationContext* sctxt)
{
    std::ifstream ifs(filename);
    if (ifs.is_open())
    {
        sstring::getRegistry().load(ifs);
        boost::archive::text_iarchive ar(ifs);
        ar & v;
//        unsigned len;
//        ar >> len;
//        while(len--)
//            v.emplace_back(sstring::String(ar, sctxt));
    }
}

void ssave(const char* filename, vector<sstring::String>& v, sstring::SerializationContext* sctxt)
{
    std::ofstream ofs(filename);
    if (ofs.is_open())
    {
        sstring::getRegistry().save(ofs);
        boost::archive::text_oarchive ar(ofs);
        ar & v;
//        unsigned len = v.size();
//        ar << len;
//
//        for(auto itr= v.begin(); itr != v.end(); ++itr)
//        {
//            itr->save(ar);
//        }
    }
}

const char *lab[] = 
{
    "abandon",
    "abandoned",
    "ability",
    "able",
    "about",
    "above",
    "abroad",
    "absence",
    0
};

const char *lcd[] = 
{
    "cabinet",
    "cable",
    "cake",
    "calculate",
    "calculation",
    0
};

const char *leg[] =
{
    "each",
    "each other",
    "ear",
    "early",
    "earn",
    "earth",
    "ease",
    "easily",
    0
};

const char** lsts[] =
{
    lab,
    lcd,
    leg
};

class SCTest
{
    private:
        sstring::SerializationContext *sctxt;
        std::string fname;
        bool changed = false;
        vector<sstring::String> strings;
   
    public:
        void l_init(const char** l)
        {
            sstring::ContextGuard cg(sctxt);
            for (int ix=0; l[ix]; ++ix)
            {
                strings.push_back(sstring::String(l[ix], sctxt));
            }
            changed = true;
        }

        SCTest(const char* f):sctxt(sstring::getRegistry().makeSerializationContext()), fname(f)
        {
            sstring::ContextGuard cg(sctxt);
            sload(fname.c_str(), strings, sctxt);
        }

        void save(void)
        {
            if (changed)
            {
                sstring::ContextGuard cg(sctxt);
                ssave(fname.c_str(), strings, sctxt);
            }
        }

        void dump(void)
        {
            std::cerr << fname << ", " << strings.size() << std::endl;
            for (auto itr = strings.begin(); itr != strings.end(); ++itr)
            {
                std::cerr << " -- " << unsigned(*itr) << " " << *itr << std::endl;
            }
        }
};

int main(int argc, char *argv[])
{
    vector<SCTest> scts;

    for(int ix = 1; ix < argc; ix += 2)
    {
        SCTest sct(argv[ix]);
        if (ix + 1 < argc)
        {
            unsigned x = std::atoi(argv[ix + 1]);
            if (x < sizeof(lsts)/sizeof(lsts[0]))
            {
                sct.l_init(lsts[x]);
            }
        }
        scts.push_back(sct);
    }

    for(auto itr = scts.begin(); itr != scts.end(); ++itr)
    {
        itr->dump();
    }

    for(auto itr = scts.begin(); itr != scts.end(); ++itr)
    {
        itr->save();
    }
}

