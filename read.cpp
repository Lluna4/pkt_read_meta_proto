#include <print>
#include <string>
#include <tuple>
#include <cstring>
#include <chrono>
#include "packet_read.hpp"

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

// Template struct for reading variables
template<typename T>
struct read_var;

// Specialization for char
template<>
struct read_var<char>
{
    static char call(char *v)
    {
        return read_char(v);
    }
};

// Specialization for unsigned char
template<>
struct read_var<unsigned char>
{
    static unsigned char call(char *v)
    {
        return read_uchar(v);
    }
};

template<>
struct read_var<int>
{
    static int call(char *v)
    {
        return read_int(v);
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
    char test[12] = {0};
    int a = 1;
    int b = 90;
    int c = 1209;
    std::memcpy(test, &a, sizeof(int));
    std::memcpy(&test[4], &b, sizeof(int));
    std::memcpy(&test[8], &c, sizeof(int));
    char *ptr = test;

    // Read a tuple of char, unsigned char, char
    const auto before = clock::now();
    std::tuple<int, int, int> t;
    constexpr std::size_t size = std::tuple_size_v<decltype(t)>;
    const_for<size>([&](auto i)
    {
        std::get<i.value>(t) = read_var<std::tuple_element_t<i.value, decltype(t)>>::call(ptr);
        ptr += sizeof(std::tuple_element_t<i.value, decltype(t)>);
    });
   
    const ms duration = clock::now() - before;
    std::cout << "It took " << duration.count() << "ms" << std::endl;
    packet p = {0, 12, 12, test};
    const auto before1 = clock::now();
   
    auto result2 = pkt_read(&p, {{{"int1", &typeid(int)}, {"int2", &typeid(int)}, {"int3", &typeid(int)}}});
    
    const ms duration1 = clock::now() - before1;
    std::cout << "It took " << duration1.count() << "ms" << std::endl;

    std::println("{}, {}, {}", std::get<0>(t), std::get<1>(t), std::get<2>(t));
    std::println("{}, {}, {}", result2.get<int>("int1"), result2.get<int>("int2"), result2.get<int>("int3"));
    return 0;
}

