#include <print>
#include <string>
#include <tuple>
#include <cstring>
#include <chrono>
#include "packet_read.hpp"
#include <frozen/map.h>
#include <frozen/string.h>

// Read a single char from a char pointer
char read_char(char *v)
{
    return *v;
}

// Read a single unsigned char from a char pointer
unsigned char read_uchar(char *v)
{
    return (unsigned char)*v;
}

int read_int(char *v)
{
    int a = 0;

    std::memcpy(&a, v, sizeof(int));

    return a;
}

unsigned short read_ushort(char *v)
{
    unsigned short  a = 0;

    std::memcpy(&a, v, sizeof(unsigned short));

    return a;
}

minecraft::string read_string_m(char *v)
{
    unsigned long s = 0;
    std::size_t varint_size = ReadUleb128(v, &s);
    std::string str;
    
    v += varint_size;
    str = v;
    str = str.substr(0, s);
    return (minecraft::string){.size = s, .str = str};
}

// Template struct for reading variables
template<typename T>
struct read_var;

// Specialization for char
template<>
struct read_var<char>
{
    static char call(char **v)
    {
        char ret = read_char(*v);
        *v++;
        return ret;
    }
};

// Specialization for unsigned char
template<>
struct read_var<unsigned char>
{
    static unsigned char call(char **v)
    {
        unsigned char ret = read_uchar(*v);
        *v++;
        return ret;
    }
};

template<>
struct read_var<int>
{
    static int call(char **v)
    {
        int ret = read_int(*v);
        *v += sizeof(int);
        return ret;
    }
};

template<>
struct read_var<varint>
{
    static varint call(char **v)
    {
        unsigned long ret = 0;
        int size = ReadUleb128(*v, &ret);
        *v += size;
        return (varint){.size = size,.num = ret};
    }
};

template<>
struct read_var<unsigned short>
{
    static unsigned short call(char **v)
    {
        unsigned short ret = read_ushort(*v);
        *v += sizeof(unsigned short);
        return ret;
    }
};

template<>
struct read_var<minecraft::string>
{
    static minecraft::string call(char **v)
    {
        minecraft::string ret = read_string_m(*v);
        *v += ret.size + 1;
        return ret;
    }
};

template <typename Integer, Integer ...I, typename F> constexpr void const_for_each(std::integer_sequence<Integer, I...>, F &&func)
{
    (func(std::integral_constant<Integer, I>{}) , ...);
}

template <auto N, typename F> constexpr void const_for(F &&func)
{
    if constexpr (N > 0)
        const_for_each(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
}


int main()
{
    using clock = std::chrono::system_clock;
    using ms = std::chrono::duration<double, std::milli>;

    std::string buf;
    unsigned short port = 25565;
    WriteUleb128(buf, 765);
    WriteUleb128(buf, strlen("hola!"));
    buf.append("hola!");
    char a[3] = {0};
    std::memcpy(a, &port, sizeof(unsigned short));
    buf.append(a);
    WriteUleb128(buf, 1);

    // Read a tuple of char, unsigned char, char
    int i = 0;
    std::vector<std::tuple<varint, minecraft::string, unsigned short, varint>> vec;
    vec.reserve(1000000);
    const auto before = clock::now();
    while (i < 1000000)
    {
        std::tuple<varint, minecraft::string, unsigned short, varint> t;
        constexpr std::size_t size = std::tuple_size_v<decltype(t)>;
        char *ptr = (char *)buf.c_str();
        const_for<size>([&](auto i)
        {
            read_var<std::tuple_element_t<i.value, decltype(t)>>::call(&ptr);
        });
        i++;
        vec.push_back(t);
    }
   
    const ms duration = clock::now() - before;
    std::cout << "It took " << duration.count() << "ms" << std::endl;
    packet p = {0, 100, 100, (char *)buf.c_str()};
    const auto before1 = clock::now();
    i = 0;
    while (i < 1000000)
    {
        auto result2 = pkt_read(p, {{{"Version", &typeid(minecraft::varint)}, {"host", &typeid(minecraft::string)}, {"port", &typeid(unsigned short)}, {"state", &typeid(minecraft::varint)}},
        {"Version", "host", "port", "state"}});
        i++;
    }
    
    const ms duration1 = clock::now() - before1;
    std::cout << "It took " << duration1.count() << "ms" << std::endl;
    std::cout << vec.size() << std::endl;
    return 0; 
}

