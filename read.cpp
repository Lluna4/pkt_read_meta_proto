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
    static unsigned char call(char *v)
    {
        return read_int(v);
    }
};
template <std::size_t N, typename Tuple>
struct OffsetHelper
{
    static constexpr std::size_t value = sizeof(std::tuple_element_t<N-1, Tuple>) + OffsetHelper<N-1, Tuple>::value;
};

template <typename Tuple>
struct OffsetHelper<0, Tuple>
{
    static constexpr std::size_t value = 0;
};

// Specialization for std::tuple to read multiple types
template <typename... Ts>
struct read_var<std::tuple<Ts...>>
{
    public:
        static std::tuple<Ts...> call(char *v)
        {
            return call_impl(v, std::index_sequence_for<Ts...>{});
        }

    private:
        template <std::size_t... Is>
        static std::tuple<Ts...> call_impl(char *v, std::index_sequence<Is...>)
        {
            return std::make_tuple(read_var<std::tuple_element_t<Is, std::tuple<Ts...>>>::call(v + OffsetHelper<Is, std::tuple<Ts...>>::value)...);
        }
};

int main()
{
    using clock = std::chrono::system_clock;
    using ms = std::chrono::duration<double, std::milli>;
    char test[8] = {0};
    int a = 1;
    int b = 90;
    std::memcpy(test, &a, sizeof(int));
    std::memcpy(&test[4], &b, sizeof(int));

    // Read a tuple of char, unsigned char, char
    const auto before = clock::now();
    auto result = read_var<std::tuple<int, int>>::call(test);
    const ms duration = clock::now() - before;
    std::cout << "It took " << duration.count() << "ms" << std::endl;

    const auto before1 = clock::now();
    packet p = {0, 8, 8, test};
    auto result2 = pkt_read(&p, {{{"int1", &typeid(int)}, {"int2", &typeid(int)}}});
    const ms duration1 = clock::now() - before1;
    std::cout << "It took " << duration1.count() << "ms" << std::endl;

    std::println("{}, {}", std::get<0>(result), std::get<1>(result));
    std::println("{}, {}", result2.get<int>("int1"), result2.get<int>("int2"));

    return 0;
}

