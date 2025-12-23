#ifndef BOOTSTRAP_HPP
#define BOOTSTRAP_HPP
#include <fstream>
#include <nlohmann/json.hpp>
#include <nadi/nadi.h>
#include "core_callbacks.hpp"
#include "threads.hpp"
#include "nadi/unique_message.hpp"
#include <vector>

void bootstrap_dispatch_to_target(nadi_unique_message msg, const nlohmann::json& target, nadi_threads_t& threads)
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
        threads.push_bootstrap_message(target_name,{std::move(msg),route_address{0,channel}});
    }
    else {
        //TODO error
    }
}

void handle_bootstrap_message(const nlohmann::json& msgs_json, nadi_threads_t& threads){
    for(auto& msg_json:msgs_json){
        if(msg_json.contains("meta") && msg_json.contains("data")){
            nadi_unique_message msg(new nadi_message);
            msg.get()->free = free_msg;
            if(msg_json.contains("channel")){
                msg.get()->channel = msg_json["channel"].get<unsigned int>();
            }
            else{
                msg.get()->channel = 0;
            }
            std::string meta = msg_json["meta"].dump();
            msg.get()->meta = new char[meta.size() + 1];
            std::strcpy((char*)msg.get()->meta, meta.c_str());
            std::string data = msg_json["data"].dump();
            msg.get()->data = new char[data.size() + 1];
            std::strcpy((char*)msg.get()->data, data.c_str());
            msg.get()->data_length = data.size();
            auto targets = msg_json["target"];
            for(const auto& target : targets){
                bootstrap_dispatch_to_target(std::move(msg),target,threads);
            }
        }
    }
    std::atomic<int> ai = threads.size();
    threads.push_bootstrap_rpc([&](){ --ai; }); //send an rpc to all threads, each will decriment when they execute it
    while(ai != 0){
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
    }
}

nlohmann::json parse_bootstrap(const std::string& bootstrap_file){
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