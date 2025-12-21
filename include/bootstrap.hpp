#ifndef BOOTSTRAP_HPP
#define BOOTSTRAP_HPP
#include <fstream>
#include <nlohmann/json.hpp>
#include <nadi/nadi.h>
#include "core_callbacks.hpp"
#include "threads.hpp"
#include <vector>

void bootstrap_dispatch_to_target(const nadi_message* msg, const nlohmann::json& target, nadi_threads_t& threads)
{
    if(target.size() == 2){
        std::string target_name = target[0];
        unsigned channel = 0;
        if(target[1].is_number()){
            channel = target[1].get<unsigned int>();
        }
        else if(target[1].is_string()){
            const std::string hex_str = target[1].get<std::string>();
            size_t pos = 0;
            channel = std::stoul(hex_str, &pos, 16);
        }
        if(target_name == "context"){
            threads.push_message_non_local(0,{route_address{0,channel},msg});
        }
        else{
            //TODO handle messages to normal nodes
        }
    }
    else {
        //TODO error
    }
}

void handle_bootstrap_message(const nlohmann::json& msg_json, nadi_threads_t& threads){
    if(msg_json.contains("meta") && msg_json.contains("data")){
        nadi_message* msg = new nadi_message;
        msg->free = free_msg;
        if(msg_json.contains("channel")){
            msg->channel = msg_json["channel"].get<unsigned int>();
        }
        else{
            msg->channel = 0;
        }
        std::string meta = msg_json["meta"].dump();
        msg->meta = new char[meta.size() + 1];
        std::strcpy((char*)msg->meta, meta.c_str());
        std::string data = msg_json["data"].dump();
        msg->data = new char[data.size() + 1];
        std::strcpy((char*)msg->data, data.c_str());
        msg->data_length = data.size();
        auto targets = msg_json["target"];
        for(const auto& target : targets){
            bootstrap_dispatch_to_target(msg,target,threads);
        }
    }
}

nlohmann::json handle_bootstrap(const std::string& bootstrap_file){
    // Process bootstrap file
    std::ifstream file(bootstrap_file);
    if (file.is_open()) {
        nlohmann::json bootstrap_json;
        file >> bootstrap_json;
        file.close();
        return bootstrap_json;
    }
    return {};
}


#endif