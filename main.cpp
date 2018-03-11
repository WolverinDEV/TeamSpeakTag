#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <cstring>
#include <json/json.h>
#include "utils.h"
#include "TSTag.hpp"

void testTagParsing() {
    //Blacklist query
    auto input = "0A04576AFCA4120B4D4347616C6178792E64651A1C4B524A464A586F396D32574971694D67674B493558596668484E673D28834E30CCF590D5054001";
    auto vec = hex::decode(input);
    auto istream = std::istringstream(std::string(vec.begin(), vec.end()));
    auto decoded = tstag::decode(istream);
    for(const auto &tag : decoded) {
        std::cout << "Index " << tag.first << " Type: " << tag.second->type() << std::endl;
        if(tag.second->type() == tstag::VAR_INT)
            std::cout << " Value: " << std::dynamic_pointer_cast<tstag::TagVarInt>(tag.second)->getValue() << std::endl;
        else if(tag.second->type() == tstag::TEXT)
            std::cout << " Value: " << std::dynamic_pointer_cast<tstag::TagString>(tag.second)->getValue() << std::endl;
    }

    auto encoded = tstag::encode(decoded);
    std::cout << "Hex:" << std::endl;
    std::cout << hex::encode(encoded) << std::endl;
    std::cout << "Equal: " << (hex::encode(encoded).compare(input) == 0 ? "yes" : "no") << std::endl;
}

#define AERR(message) \
do {\
    cerr << "Invalid argument! (" << message << ")" << endl; \
    return 1; \
} while(0)

using namespace std;
int main(int argc, char** argv) {

    bool decode;
    if(strcmp(argv[1], "decode") == 0)
        decode = true;
    else if(strcmp(argv[1], "encode") == 0)
        decode = false;
    else AERR("Argument one must be decode or encode");

    string source;

    if(strcmp(argv[2], "cin") == 0) {
        string buffer;
        while(std::getline(cin, buffer)) source += buffer + "\n";
        source.substr(0, source.length() - 1); //Cut off the newline delimiter
    } else if(strcmp(argv[2], "hex") == 0) {
        if(!decode) AERR("You only could applay a hex in decode mode!");
        source = argv[3];
    } else if(strcmp(argv[2], "json") == 0) {
        if(decode) AERR("You only could applay a json in encode mode!");
        source = argv[3];
    } else AERR("Argument two must be cin or hex");


    if(decode) { //decode
        auto bin = hex::decode(source);
        if(bin.empty())
            AERR("Invalid hex string! (decode failed)");
        istringstream is(string(bin.begin(), bin.end()));
        auto result = tstag::decode(is);
        if(result.empty())
            AERR("Invalid hex string (empty result)");

        Json::Value root(Json::ValueType::arrayValue);
        for(const auto& tag : result) {
            Json::Value tagRoot;
            tagRoot["index"] = tag.first;
            tagRoot["type"] = tag.second->type();
            if(tag.second->type() == tstag::VAR_INT)
                tagRoot["value"] = dynamic_pointer_cast<tstag::TagVarInt>(tag.second)->getValue();
            else if(tag.second->type() == tstag::TEXT)
                tagRoot["value"] =  dynamic_pointer_cast<tstag::TagString>(tag.second)->getValue();
            root.append(tagRoot);
        }

        Json::FastWriter writer{};
        cout << writer.write(root) << endl;
        return 0;
    } else {
        Json::Reader reader{};
        Json::Value root;

        if(!reader.parse(source, root)) AERR("Invalid json! (parse: " + reader.getFormattedErrorMessages() + ")");
        if(root.type() != Json::ValueType::arrayValue) AERR("Invalid json (type)");

        std::map<int, std::shared_ptr<tstag::Tag>> tags;
        for(const auto& tagRoot : root) {
            std::shared_ptr<tstag::Tag> tag;
            if(tagRoot["type"].asInt() == tstag::TEXT) {
                tag = std::make_shared<tstag::TagString>(tagRoot["value"].asString());
            } else if(tagRoot["type"].asInt() == tstag::VAR_INT) {
                tag = std::make_shared<tstag::TagVarInt>(tagRoot["value"].asInt64());
            }
            tags[tagRoot["index"].asInt()] = tag;
        }

        auto encoded = tstag::encode(tags);
        cout << hex::encode(encoded) << endl;
        return 0;
    }

    return 0;
}