#pragma once

#include <cstdint>
#include <sstream>
#include <iostream>
#include <deque>
#include <cassert>

namespace hex {
    inline uint8_t decodeNibble(std::istream &stream) {
        uint8_t c;
        stream >> c;
        if(c >= '0' && c <= '9')
            return c - '0';
        if(c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        if(c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        std::cerr << "decodeHexNibble() => invalid character (" << (char) c << ")" << std::endl;
        return 0;
    }

    inline char encodeNibble(uint8_t c) {
        if(c >= 0 && c <= 9) return '0' + c;
        if(c >= 10 && c <= 15) return 'A' + c - 10;
        std::cerr << "encodeHexNibble() => invalid number (" << c << ")" << std::endl;
        return '0';
    }

    inline std::deque<uint8_t> decode(const std::string &input) {
        assert(input.length() % 2 == 0);
        std::deque<uint8_t> result{};
        std::istringstream stream(input);
        size_t length = input.length() / 2;
        while(length-- > 0) {
            uint8_t number = 0;
            number |= (decodeNibble(stream) << 4) & 0xF0;
            number |= (decodeNibble(stream) << 0) & 0x0F;
            result.push_back(number);
        }
        return result;
    }

    inline std::string encode(const std::deque<uint8_t>& input) {
        std::ostringstream stream;
        for(const auto& c : input) {
            stream << encodeNibble((c >> 4) & 0xF);
            stream << encodeNibble((c >> 0) & 0xF);
        }
        return stream.str();
    }

    inline std::string encode(const std::string& input) {
        std::deque<uint8_t> list(input.begin(), input.end());
        return encode(list);
    }
}

namespace varint {
    template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
    inline T decode(std::istream& stream, size_t maxLen = sizeof(T)) {
        size_t index = 0;
        T result = 0;
        uint8_t read = 0;
        do {
            if(!stream.read((char*) &read, 1)){
                std::cerr << "readVarInt() => stream underflow!" << std::endl;
                return 0;
            }
            if(index * 7 >= maxLen * 8)
                std::cerr << "readVarInt() => buffer underflow!" << std::endl;
            else
                result |= ((T)(read & 0x7F) << (index++ * 7));
        } while (read & 0x80);
        return result;
    };


    template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
    inline size_t encode(std::ostream& stream, T number) {
        size_t bytes = 0;
        while (number > 0x7F) {
            bytes++;
            stream << (uint8_t) ((number & 0x7F) | 0x80);
            number = (number >> 7);
        }

        stream << (uint8_t) number;
        return bytes + 1;
    };
}