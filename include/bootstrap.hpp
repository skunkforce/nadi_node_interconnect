#ifndef BOOTSTRAP_HPP
#define BOOTSTRAP_HPP
#include <fstream>
#include <nlohmann/json.hpp>
#include "nadi.h"
#include "core_callbacks.hpp"
#include "management.hpp"

void handle_bootstrap(const std::string& bootstrap_file, management& mngr){
    // Process bootstrap file
    std::ifstream file(bootstrap_file);
    if (file.is_open()) {
        nlohmann::json bootstrap_json;
        file >> bootstrap_json;
        file.close();
        auto messages = bootstrap_json["messages"];
        for (const auto& msg_json : messages) {
            nadi_message* msg = new nadi_message;
            msg->free = free_msg;
            msg->channel = msg_json["channel"].get<unsigned int>();
            std::string meta = msg_json["meta"].dump();
            msg->meta = new char[meta.size() + 1];
            std::strcpy(msg->meta, meta.c_str());
            std::string data = msg_json["data"].dump();
            msg->data = new char[data.size() + 1];
            std::strcpy(msg->data, data.c_str());
            msg->data_length = data.size();
            mngr.callback(msg); // Process via management::callback
        }
    }
}
#endif