#include <iostream>
#include <vector>
#include <algorithm>


bool compare(const int a, const int b)
{
    return b < a;
}

int main( int argc, char* argv[] )
{
    int data[] = {5,2,1,4,0};
    std::vector<int> v(data, data + (sizeof(data)/sizeof(data[0])));

    for(std::vector<int>::iterator itr=v.begin(); itr != v.end(); ++itr)
    {
      std::cout << *itr;
    }
    std::cout << std::endl;

    sort(v.begin(), v.end());
    for(std::vector<int>::iterator itr=v.begin(); itr != v.end(); ++itr)
    {
      std::cout << *itr;
    }
    std::cout << std::endl;

    sort(v.begin(), v.end(), compare);
    for(std::vector<int>::iterator itr=v.begin(); itr != v.end(); ++itr)
    {
      std::cout << *itr;
    }
    std::cout << std::endl;
}
