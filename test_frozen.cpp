#include <frozen/map.h>
#include <frozen/string.h>
#include <iostream>

int main()
{
    constexpr frozen::map<int, frozen::string, 2> test = {{1, "test1"}, {2, "aa"}};

    std::cout << test.find(1)->second.data() << " " << test.find(2)->second.data()<< std::endl;
}