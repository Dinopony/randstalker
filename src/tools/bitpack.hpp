#pragma once

#include <landstalker_lib/tools/json.hpp>
#include <landstalker_lib/tools/bitfield.hpp>

class Bitpack;
template<typename T> T unpack_from(Bitpack& bitpack);

class Bitpack
{
private:
    std::vector<uint8_t> _bytes;
    Bitfield _bits;

    size_t _current_byte_index;
    size_t _current_bit_index;
    
public:
    Bitpack() :
        _current_byte_index(0),
        _current_bit_index(0)
    {}

    explicit Bitpack(const std::vector<uint8_t>& bytes) : Bitpack()
    {
        uint8_t bits_count = bytes[0];
        uint8_t bit_bytes_size = bits_count / 8;
        if(bits_count % 8 > 0)
            bit_bytes_size++;

        std::vector<uint8_t> bit_bytes(bytes.begin() + 1, bytes.begin() + 1 + bit_bytes_size);
        _bits = Bitfield(bit_bytes, bits_count);

        _bytes = std::vector<uint8_t>(bytes.begin() + 1 + bit_bytes_size, bytes.end());
    }

    [[nodiscard]] std::vector<uint8_t> to_bytes() const
    {
        std::vector<uint8_t> ret;
        
        ret.emplace_back(_bits.size());
        std::vector<uint8_t> bit_bytes = _bits.to_bytes();
        ret.insert(ret.end(), bit_bytes.begin(), bit_bytes.end());

        ret.insert(ret.end(), _bytes.begin(), _bytes.end());

        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    void pack_pod_type(T& value)
    {
        uint8_t* value_as_bytes = reinterpret_cast<uint8_t*>(&value);
        std::vector<uint8_t> value_as_bytes_vec(value_as_bytes, value_as_bytes + sizeof(T));
        _bytes.insert(_bytes.end(), value_as_bytes_vec.begin(), value_as_bytes_vec.end());
    }

    template<typename T>
    void pack_vector(const std::vector<T>& vector)
    {
        this->pack((uint16_t)vector.size());
        for(const T& elem : vector)
            this->pack(elem);
    }

    template<typename K, typename V>
    void pack_map(const std::map<K,V>& map)
    {
        this->pack((uint16_t)map.size());
        for(auto& [k,v] : map)
        {
            this->pack((K)k);
            this->pack((V)v);
        }
    }

    void pack(uint8_t value)       { this->pack_pod_type(value); }
    void pack(uint16_t value)      { this->pack_pod_type(value); }
    void pack(uint32_t value)      { this->pack_pod_type(value); }
    void pack(float value)         { this->pack_pod_type(value); }
    void pack(double value)        { this->pack_pod_type(value); }
    void pack(bool value)          { _bits.add(value); }

    void pack(const std::string& value)
    {
        for(uint8_t character : value)
            this->pack(character);
        this->pack((uint8_t) '\0');
    }

    void pack(const Json& json)
    {
        this->pack_vector(Json::to_msgpack(json));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////

    uint8_t* pointer_on_current_byte()
    {
        return &(_bytes[_current_byte_index]);
    }

    void increment_byte_pointer(size_t type_size)
    {
        _current_byte_index += type_size;
    }

    template<typename T>
    T unpack()
    {
        return unpack_from<T>(*this);
    }

    bool unpack_bit()
    {
        return _bits.get((uint32_t)_current_bit_index++);
    }

    template<typename T>
    std::vector<T> unpack_vector()
    {
        std::vector<T> ret;

        uint16_t vector_size = this->unpack<uint16_t>();
        for(uint16_t i=0 ; i<vector_size ; ++i)
            ret.emplace_back(this->unpack<T>());

        return ret;
    }

    template<typename K, typename V>
    std::map<K,V> unpack_map()
    {
        std::map<K,V> ret;

        uint16_t map_size = this->unpack<uint16_t>();
        for (uint16_t i = 0; i < map_size; ++i)
        {
            K key = this->unpack<K>();
            V value = this->unpack<V>();
            ret[key] = value;
        }

        return ret;
    }
};

template<typename T>
T unpack_from(Bitpack& bitpack)
{
    T value = *reinterpret_cast<T*>(bitpack.pointer_on_current_byte());
    bitpack.increment_byte_pointer(sizeof(T));
    return value;
}

template<>
std::string unpack_from<std::string>(Bitpack& bitpack)
{
    uint8_t* string_bytes = bitpack.pointer_on_current_byte();
    std::string string(reinterpret_cast<const char*>(string_bytes));

    bitpack.increment_byte_pointer(string.size() + 1);
    return string;
}

template<>
Json unpack_from<Json>(Bitpack& bitpack)
{
    std::vector<uint8_t> json_bytes = bitpack.unpack_vector<uint8_t>();
    return Json::from_msgpack(json_bytes);
}

template<>
bool unpack_from<bool>(Bitpack& bitpack)
{
    return bitpack.unpack_bit();
}
