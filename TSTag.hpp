#pragma once

#include <memory>
#include <iostream>
#include <map>
#include "utils.h"

namespace tstag {
    enum TagType : uint8_t {
            VAR_INT,
            UNKNOWN_1,
            TEXT,
            UNKNOWN_3,
            UNKNOWN_4,
            UNKNOWN_5,
            UNKNOWN_6,
            UNKNOWN_7
    };

    class Tag {
        public:
            Tag(TagType type) : _type(type) {}

            virtual TagType type() const { return this->_type; }
            virtual size_t write(std::ostream&) const = 0;
            virtual bool read(std::istream&) = 0;
        private:
            TagType _type;
    };

    class TagVarInt : public Tag {
        public:
            TagVarInt() : Tag(TagType::VAR_INT), value(0) {}
            TagVarInt(const int64_t& value) : Tag(TagType::VAR_INT), value(value) {}

            size_t write(std::ostream &ostream1) const override {
                return varint::encode(ostream1, value);
            }

            bool read(std::istream &istream1) override {
                value = varint::decode<int64_t>(istream1);
                return !!istream1;
            }

            int64_t getValue() const { return value; }
        private:
            int64_t value;
    };

    class TagString : public Tag {
        public:
            TagString() : Tag(TagType::TEXT), value("") {}
            TagString(std::string value) : Tag(TagType::TEXT), value(std::move(value)) {}

            size_t write(std::ostream &stream) const override {
                size_t length = varint::encode(stream, value.length());
                stream << value;
                return length + value.length();
            }

            bool read(std::istream &stream) override {
                auto length = varint::decode<size_t>(stream);
                //auto length = stream.get();
                value.resize(length);
                if(!stream.read(&value[0], length)) {
                    std::cerr << "TagString::read() => stream underflow!" << std::endl;
                    return false;
                }
                return !!stream;
            }

            std::string getValue() const { return this->value; }
        private:
            std::string value;
    };

    inline std::map<int, std::shared_ptr<Tag>> decode(std::istream& stream) {
        std::map<int, std::shared_ptr<Tag>> tags;

        while(stream) {
            uint8_t head;
            if(!stream.read((char*) &head, 1)) break;

            auto type = (uint8_t) (head & 0x7);
            uint8_t index = head >> 3;

            std::shared_ptr<Tag> tag;
            if(type == TagType::TEXT)
                tag = std::make_shared<TagString>();
            else if(type == TagType::VAR_INT)
                tag = std::make_shared<TagVarInt>();
            else {
                std::cerr << "Invalid tag type (" << (int) type << ")!" << std::endl;
                return {};
            }
            if(!tag->read(stream)) {
                std::cerr << "Could not read tag!" << std::endl;
                return {};
            }

            tags[index] = tag;
        }

        return tags;
    }

    inline std::string encode(const std::map<int, std::shared_ptr<Tag>>& tags) {
        std::stringstream result;

        for(const auto& tag : tags) {
            uint8_t head = 0;
            assert(tag.first < 0xF8);

            head |= tag.first << 3;
            head |= tag.second->type() & 0x07;

            result << head;
            tag.second->write(result);
        }

        return result.str();
    }
}