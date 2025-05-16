#ifndef NODE_MANAGEMENT_HPP
#define NODE_MANAGEMENT_HPP
#include <nlohmann/json.hpp>
#include "nadi.h"
#include <vector>
#include <tuple>
#include "find_nodes.hpp"

class node_management{
    std::vector<std::tuple<nadi_library,std::vector<std::tuple<std::string,void*>>,std::filesystem::path>> nodes_;
    public:
    void load_nodes(std::string nodes_dir){
        auto node_paths_ = get_node_paths(nodes_dir);
        for(const auto &path: node_paths_){
            nodes_.push_back({load_node(path.string()),{},path});
        }
    }
    nlohmann::json to_json(){
        nlohmann::json node_json;
        node_json["meta"] = nlohmann::json::object();
        node_json["meta"]["type"] = "description_of_known_nodes";
        node_json["data"] = nlohmann::json::array();
        for(const auto& [node,conns,path]: nodes_){
            
            auto description = std::string(node.descriptor());
            auto obj = nlohmann::json::object();
            obj["filename"] = path.filename().string();
            obj["description"] = nlohmann::json::parse(description);
            node_json["data"].push_back(obj);
        }
        return node_json;
    }
};

#endif