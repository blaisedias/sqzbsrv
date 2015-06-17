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

vector<sstring::String> strings;

int main(int argc, char *argv[])
{
    sstring::load("data/exp_cchars.dat");
if(cc_dump)
    sstring::dump();
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
    sstring::prune();
if(cc_dump)
    sstring::dump();

    if (true)
    {
std::cout << "== II  " << std::endl;
        sstring::String zambia("Zambia");
        sstring::String aloha("Aloha");
        sstring::String aardvark("Aardvark");
        sstring::String *namaste = new sstring::String("Namaste");
        sstring::String *zambia2 = new sstring::String("Zambia");

        std::cout << zambia << std::endl;
        std::cout << aloha << std::endl;
        std::cout << aardvark << std::endl;
        std::cout << *namaste << std::endl;
        std::cout << *zambia2 << std::endl;
        delete zambia2;
        
if(cc_dump)
    sstring::dump();

        sstring::save("data/exp_cchars.dat");
    }

    std::cout << "---------------" << std::endl;
if(cc_dump)
    sstring::dump();
    if(true)
    {
std::cout << "== III  " << std::endl;
        if (strings.size() < 20)
        {
            std::cout << "1" << std::endl;
            strings.push_back(sstring::String("Zambia"));
            std::cout << "2" << std::endl;
            strings.push_back(sstring::String("Aloha"));
            std::cout << "3" << std::endl;
            strings.push_back(sstring::String("Aardvark"));
            std::cout << "4" << std::endl;
            strings.push_back(sstring::String("Namaste"));
            std::cout << "5" << std::endl;
            strings.push_back(sstring::String("Zambia"));
            std::cout << "6" << std::endl;
            strings.push_back(sstring::String("Zonda"));
        }

        for(unsigned ix=0; ix < strings.size(); ix++)
        {
            std::cout << strings[ix] << std::endl;
        }
        sstring::save("data/exp_cchars.dat");
        
        std::ofstream ofs("data/exp_sstrings.dat");
        if (ofs.is_open())
        {
            boost::archive::text_oarchive ar(ofs);
        
            ar & strings;
        }
        cout << "Sizeof sstring " << sizeof(strings[0]) << "\n";
    }
//    strings.clear();
    cout << "Fini" << "\n";
}


