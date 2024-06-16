#include "utils.hpp"
#include <vector>
#include <map>
#include <any>
#include <string>
#include <cstring>
#include <string.h>
#include <print>
#include <algorithm>
#if defined(__linux__)
#  include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#  define be64toh(x) betoh64(x)
#endif

class Packet
{
    public:
        Packet(std::map<std::string, std::any> data)
            :data_(data)
            {}
        
        template<typename T>
        T get(std::string key) const 
        {
            auto val = data_.find(key);

            if (val == data_.end())
            {
                throw std::runtime_error("Bad key!");
            }
            else
            {
                return std::any_cast<T>(val->second);
            }
        }
    private:
        std::map<std::string, std::any> data_;
};

template<class A>
class Packet_c
{
    public:
        Packet_c(std::vector<std::string> names, std::vector<A> t)
            :names_(names), t_(t)
            {}
        
        int get_index(std::string name)
        {
            return std::distance(names_.begin(), std::find(names_.begin(), names_.end(), name));
        }

        A get(std::string name)
        {
            return t_[std::distance(names_.begin(), std::find(names_.begin(), names_.end(), name))];
        }
    private:
        std::vector<std::string> names_;
        std::vector<A> t_;
};

template <class Tuple,
   class T = std::decay_t<std::tuple_element_t<0, std::decay_t<Tuple>>>>
std::vector<T> to_vector(Tuple&& tuple)
{
    return std::apply([](auto&&... elems){
        return std::vector<T>{std::forward<decltype(elems)>(elems)...};
    }, std::forward<Tuple>(tuple));
}

Packet pkt_read(packet *p, indexed_map types)
{
    std::map<std::string, std::any> ret;
    int initial_size = p->buf_size;
    int size = p->buf_size;
    char *data = p->data;


    for (int i = 0; i < types.map.size(); i++)
    {
        auto var = types.map[i];
        if (var.type->hash_code() == typeid(char).hash_code())
        {
            if (size < sizeof(char))
                break;
            ret.insert({ types.map[i].name, *data});
            data++;
            size--;
        }
        else if (var.type->hash_code() == typeid(unsigned char).hash_code())
        {
            if (size < sizeof(unsigned char))
                break;
            ret.insert({types.map[i].name, (unsigned char)*data});
            data++;
            size--;
        }
        else if (var.type->hash_code() == typeid(bool).hash_code())
        {
            if (size < sizeof(bool))
                break;
            ret.insert({types.map[i].name, (bool)*data});
            data++;
            size--;
        }
        else if (var.type->hash_code() == typeid(short).hash_code())
        {
            if (size < sizeof(short))
                break;
            short num = 0;

            std::memcpy(&num, data, sizeof(short));
            //num = be16toh(num);
            ret.insert({types.map[i].name, num});
            data += 2;
            size -= 2;
        }
        else if (var.type->hash_code() == typeid(unsigned short).hash_code())
        {
            if (size < sizeof(unsigned short))
                break;
            unsigned short num = 0;

            std::memcpy(&num, data, sizeof(unsigned short));
            //num = be16toh(num);
            ret.insert({types.map[i].name, num});
            data += 2;
            size -= 2;
        }
        else if (var.type->hash_code() == typeid(int).hash_code())
        {
            if (size < sizeof(int))
                break;
            int num = 0;

            std::memcpy(&num, data, sizeof(int));
            //num = be32toh(num);
            ret.insert({types.map[i].name, num});
            data += 4;
            size -= 4;
        }
        else if (var.type->hash_code() == typeid(unsigned int).hash_code())
        {
            if (size < sizeof(unsigned int))
                break;
            unsigned int num = 0;

            std::memcpy(&num, data, sizeof(unsigned int));
            //num = be32toh(num);
            ret.insert({types.map[i].name, num});
            data += 4;
            size -= 4;
        }
        else if (var.type->hash_code() == typeid(long).hash_code())
        {
            if (size < sizeof(long))
                break;
            long num = 0;

            std::memcpy(&num, data, sizeof(long));
            //num = be64toh(num);
            ret.insert({types.map[i].name, num});
            data += 8;
            size -= 8;
        }
        else if (var.type->hash_code() == typeid(unsigned long).hash_code())
        {
            if (size < sizeof(unsigned long))
                break;
            unsigned long num = 0;

            std::memcpy(&num, data, sizeof(unsigned long));
            //num = be64toh(num);
            ret.insert({types.map[i].name, num});
            data += 8;
            size -= 8;
        }
        else if (var.type->hash_code() == typeid(float).hash_code())
        {
            if (size < sizeof(float))
                break;
            ret.insert({types.map[i].name, read_float(data)});
            data += 4;
            size -= 4;
        }
        else if (var.type->hash_code() == typeid(double).hash_code())
        {
            if (size < sizeof(double))
                break;

            ret.insert({types.map[i].name, read_double(data)});
            data += 8;
            size -= 8;
        }
    }
    //std::println("Header size {}", initial_size - size);
    p->data = data;
    return Packet(ret);
}

std::vector<Packet> pkt_read(packet* p, indexed_map types, int times)
{
    std::vector<Packet> rett;
    int initial_size = p->buf_size;
    int size = p->buf_size;
    char* data = p->data;

    for (int x = 0; x < times; x++)
    {
        std::map<std::string, std::any> ret;
        for (int i = 0; i < types.map.size(); i++)
        {
            auto var = types.map[i];
            if (var.type->hash_code() == typeid(char).hash_code())
            {
                if (size < sizeof(char))
                    break;
                ret.insert({ types.map[i].name, *data });
                data++;
                size--;
            }
            else if (var.type->hash_code() == typeid(unsigned char).hash_code())
            {
                if (size < sizeof(unsigned char))
                    break;
                ret.insert({ types.map[i].name, (unsigned char)*data });
                data++;
                size--;
            }
            else if (var.type->hash_code() == typeid(bool).hash_code())
            {
                if (size < sizeof(bool))
                    break;
                ret.insert({ types.map[i].name, (bool)*data });
                data++;
                size--;
            }
            else if (var.type->hash_code() == typeid(short).hash_code())
            {
                if (size < sizeof(short))
                    break;
                short num = 0;

                std::memcpy(&num, data, sizeof(short));
                //num = be16toh(num);
                ret.insert({ types.map[i].name, num });
                data += 2;
                size -= 2;
            }
            else if (var.type->hash_code() == typeid(unsigned short).hash_code())
            {
                if (size < sizeof(unsigned short))
                    break;
                unsigned short num = 0;

                std::memcpy(&num, data, sizeof(unsigned short));
                //num = be16toh(num);
                ret.insert({ types.map[i].name, num });
                data += 2;
                size -= 2;
            }
            else if (var.type->hash_code() == typeid(int).hash_code())
            {
                if (size < sizeof(int))
                    break;
                int num = 0;

                std::memcpy(&num, data, sizeof(int));
                //num = be32toh(num);
                ret.insert({ types.map[i].name, num });
                data += 4;
                size -= 4;
            }
            else if (var.type->hash_code() == typeid(unsigned int).hash_code())
            {
                if (size < sizeof(unsigned int))
                    break;
                unsigned int num = 0;

                std::memcpy(&num, data, sizeof(unsigned int));
                //num = be32toh(num);
                ret.insert({ types.map[i].name, num });
                data += 4;
                size -= 4;
            }
            else if (var.type->hash_code() == typeid(long).hash_code())
            {
                if (size < sizeof(long))
                    break;
                long num = 0;

                std::memcpy(&num, data, sizeof(long));
                //num = be64toh(num);
                ret.insert({ types.map[i].name, num });
                data += 8;
                size -= 8;
            }
            else if (var.type->hash_code() == typeid(unsigned long).hash_code())
            {
                if (size < sizeof(unsigned long))
                    break;
                unsigned long num = 0;

                std::memcpy(&num, data, sizeof(unsigned long));
                //num = be64toh(num);
                ret.insert({ types.map[i].name, num });
                data += 8;
                size -= 8;
            }
            else if (var.type->hash_code() == typeid(float).hash_code())
            {
                if (size < sizeof(float))
                    break;
                ret.insert({ types.map[i].name, read_float(data) });
                data += 4;
                size -= 4;
            }
            else if (var.type->hash_code() == typeid(double).hash_code())
            {
                if (size < sizeof(double))
                    break;

                ret.insert({ types.map[i].name, read_double(data) });
                data += 8;
                size -= 8;
            }
            else if (var.type->hash_code() == typeid(driver_string).hash_code())
            {
                std::string buf;
                if (size < 48)
                    break;
                data++;
                for (int i = 0; i < 47; i++)
                {
                    buf.push_back(*data);
                    data++;
                }
                driver_string str = { .len = 48, .name = buf };
                ret.insert({ types.map[i].name, str});
            }
        }
        rett.emplace_back(ret);
    }
    //std::println("Header size {}", initial_size - size);
    p->data = data;
    return rett;
}