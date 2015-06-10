// istream::get example
#include <iostream>     // std::cin, std::cout
#include <fstream>      // std::ifstream

void read_entry(std::ifstream& ifs)
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

    std::cout << id << " " << slen << " " << chars << std::endl;
}

int main () {
//  char str[256];

//  std::cout << "Enter the name of an existing text file: ";
//  std::cin.get (str,256);    // get c-string

//  std::ifstream ifs(str);     // open file

//  char c;
//  while (ifs.get(c))          // loop getting single characters
//   std::cout << c;

  std::ifstream ifs("songs_db_cchars.dat");     // open file
  unsigned cc_id;
  unsigned cc_map_size;

  ifs >> cc_id;
  ifs >> cc_map_size;
  std::cout << cc_id << " " << cc_map_size << std::endl;

  while(cc_map_size--)
      read_entry(ifs);

  ifs.close();                // close file

  return 0;
}
