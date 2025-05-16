#include <format>
#include <string>
#include "CLI/CLI.hpp"
#include <nlohmann/json.hpp>
#include "node.hpp"
#include <print>
#include "management.hpp"
#include <expected>


management mngr;

extern "C" {
    void callback(nadi_message* msg){
        mngr.callback(msg);
    }

    void free_msg(nadi_message* msg){
        delete[] msg->data;
        delete[] msg->meta;
        delete msg;
    }
}


std::expected<nlohmann::json, std::string> read_json_from_cin() {
    std::stringstream buffer;
    std::string line;
    while (std::getline(std::cin, line)) {
        buffer << line << '\n';
    }

    std::string input = buffer.str();
    if (input.empty()) {
        return std::unexpected("Empty input");
    }

    try {
        return nlohmann::json::parse(input);
    } catch (const nlohmann::json::parse_error& e) {
        return std::unexpected(std::format("Parse error: {}", e.what()));
    }
}

int main(int argc, char **argv) {
    const std::string nodes_dir = "./nodes";
    mngr.load_nodes(nodes_dir);
    std::print("nodes:{}",mngr.to_json().dump());
    while(1){
        auto json = *read_json_from_cin();
        if(json.contains("meta") && json.contains("data") && json.contains("channel")){
            nadi_message* msg = new nadi_message;
            msg->free = free_msg;
            msg->channel = json["channel"];
            if(json["meta"].contains("format")){
                std::string meta = json["meta"].dump();
                msg->meta = new char[meta.size()+1];
                strcpy(msg->meta,meta.c_str());
                if(json["meta"]["format"] == "json"){
                    std::string data = json["data"].dump();
                    msg->data = new char[data.size()+1];
                    strcpy(msg->data,data.c_str());
                }
            }
        }
    }
    return 0;
}