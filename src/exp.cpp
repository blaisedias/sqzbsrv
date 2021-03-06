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
#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << (x) ) ).str()
using namespace std;

// for serialisation
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <thread>

#include "sstring.h"
const bool cc_dump=false;

#define CC_DUMP() if (cc_dump) {  sstring::getRegistry().dump(); }

vector<sstring::String> strings;

void testSC(void);
void test0(void);

int main(int argc, char *argv[])
{
    std::thread t1(test0);
    t1.join();
    test0();
    cout << "main TID " << std::this_thread::get_id() << "\n";
}

void test0(void)
{
    sstring::SerializationContext *sctxt = sstring::getRegistry().makeSerializationContext();
    sstring::ContextGuard ctxtg(sctxt);

    std::string stdaardvark("Aardvark");
    sstring::getRegistry().load("data/exp_cchars.dat");

    CC_DUMP();

    strings.reserve(100);
    if (true)
    {
std::cout << "== {{  loading strings" << std::endl;
        std::ifstream ifs("data/exp_sstrings.dat");
        if (ifs.is_open())
        {
            boost::archive::text_iarchive ar(ifs);
        
            ar & strings;
        }
std::cout << "== }}  loading strings" << std::endl;
    }

std::cout << "== prune test  " << std::endl;
    sstring::String *ctcs = new sstring::String("Can't touch this can't see this !!!!!!!!");
    std::cout << *ctcs << std::endl;
    delete ctcs;
    sstring::getRegistry().prune();

    CC_DUMP();


    if (true)
    {
std::cout << "== Phase II  " << std::endl;
        sstring::String zambia("Zambia");
        sstring::String aloha("Aloha");
        sstring::String aardvark(stdaardvark);
        sstring::String *namaste = new sstring::String("Namaste");
        sstring::String *zambia2 = new sstring::String("Zambia");

        std::cout << zambia << std::endl;
        std::cout << aloha << std::endl;
        std::cout << aardvark << std::endl;
        std::cout << *namaste << std::endl;
        std::cout << *zambia2 << std::endl;
        delete zambia2;
        

        CC_DUMP();


        sstring::getRegistry().save("data/exp_cchars.dat");
    }

    std::cout << "---------------" << std::endl;

    CC_DUMP();

    if(true)
    {
std::cout << "== Phase III  (" << strings.size() << ")" << std::endl;
        if (strings.size() < 20)
        {
            std::cout << "1" << std::endl;
            strings.push_back(sstring::String("Zambia"));
            std::cout << "2" << std::endl;
            strings.push_back(sstring::String("Aloha"));
            std::cout << "3" << std::endl;
            strings.push_back(stdaardvark);
            std::cout << "4" << std::endl;
            strings.push_back(sstring::String("Namaste"));
            std::cout << "5" << std::endl;
            strings.push_back(sstring::String("Zambia"));
            std::cout << "6" << std::endl;
            strings.push_back("Zonda");
        }

std::cout << "(" << strings.size() << ")" << std::endl;
        for(unsigned ix=0; ix < strings.size(); ix++)
        {
            std::cout << strings[ix] << std::endl;
        }
        sstring::getRegistry().save("data/exp_cchars.dat");

        CC_DUMP();

        
        std::ofstream ofs("data/exp_sstrings.dat");
        if (ofs.is_open())
        {
            boost::archive::text_oarchive ar(ofs);
        
            ar & strings;
        }
        cout << "Sizeof sstring " << sizeof(strings[0]) << "\n";
        cout << "Sizeof std::string " << sizeof(std::string("abcdef")) << "\n";
    }
//    strings.clear();
    cout << "Fini" << "\n";
    cout << "TID " << std::this_thread::get_id() << "\n";
}

#include <unordered_map>
typedef std::unordered_multimap<sstring::String, sstring::String> INFOMAP;
static INFOMAP infomap;
typedef std::pair<sstring::String, sstring::String> INFOMAP_PAIR;
typedef std::tuple<sstring::String, sstring::SerializationContext*> INFOMAP_TUP;
static sstring::SerializationContext *sctxt;

void testSC()
{
    sctxt = sstring::getRegistry().makeSerializationContext();
    sstring::String s1("abc", sctxt);
    std::cerr << s1 << "\n";
    infomap.emplace(s1, "def");
    infomap.emplace("ghi", "jkl");
    sstring::String s2("mno", sctxt);
    sstring::String s3("pqr", sctxt);
    infomap.emplace(s2, s3);
    infomap.insert(std::pair<sstring::String, sstring::String>("stu", "vwx"));
    infomap.insert(std::pair<sstring::String, sstring::String>({"ABC", sctxt}, "DEF"));
    infomap.insert(std::pair<sstring::String, sstring::String>({"GHI", sctxt}, {"JKL", sctxt}));
    infomap.emplace(std::pair<sstring::String, sstring::String>({"MNO", sctxt}, {"PQR", sctxt}));
    infomap.emplace(sstring::String("STU", sctxt), sstring::String("VWX", sctxt));

    for (auto itr = infomap.begin(); itr != infomap.end(); ++itr)
        std::cerr << itr->first << " ; " << itr->second << "\n";
}

