#ifndef NODE_MANAGEMENT_HPP
#define NODE_MANAGEMENT_HPP
#include <nlohmann/json.hpp>
#include "nadi.h"
#include <vector>
#include <tuple>
#include "find_nodes.hpp"
#include "core_callbacks.hpp"

class node_management{
    std::vector<std::tuple<nadi_library,std::vector<std::tuple<std::string,void*>>,std::filesystem::path>> nodes_;
    public:
    void load_nodes(std::string nodes_dir){
        auto node_paths_ = get_node_paths(nodes_dir);
        for(const auto &path: node_paths_){
            nodes_.push_back({load_node(path.string()),{},path});
        }
    }
    void construct_node(const std::string& node_name, const std::string& instance, nadi_instance_handle source_instance) {
        nadi_library lib = load_node("./nodes/" + node_name + ".dll"); // Adjust extension
        if (lib.dll) {
            nadi_instance_handle node_instance;
            lib.init(&node_instance, callback);
            instance_map_[instance] = node_instance;
            library_map_[node_instance] = lib;
            // TODO: Send confirmation response
        }
    }
    void destruct_node(const std::string& instance, nadi_instance_handle source_instance) {
        auto it = instance_map_.find(instance);
        if (it != instance_map_.end()) {
            auto& node_instance = it->second;
            library_map_[node_instance].deinit(node_instance);
            instance_map_.erase(it);
            library_map_.erase(node_instance);
            // TODO: Send confirmation response
        }
    }
    void send_loaded_list(nadi_instance_handle source_instance) {
        nlohmann::json response = {
            {"meta", {{"format", "json"}}},
            {"data", {
                {"type", "nodes.loaded.list"},
                {"nodes", to_json()["nodes"]}
            }}
        };
        send_response(source_instance, response);
    }
    void send_instances_list(nadi_instance_handle source_instance) {
        nlohmann::json instances = nlohmann::json::array();
        for (const auto& [instance, data] : instance_map_) {
            instances.push_back({{"instance", instance}});
        }
        nlohmann::json response = {
            {"meta", {{"format", "json"}}},
            {"data", {
                {"type", "nodes.instances.list"},
                {"instances", instances}
            }}
        };
        send_response(source_instance, response);
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
    nadi_library lib_from_instance(nadi_instance_handle h){
        return library_map_[h];
    }
    private:
    std::map<std::string, nadi_instance_handle> instance_map_;
    std::map<nadi_instance_handle,nadi_library> library_map_;
    void send_response(nadi_instance_handle instance, const nlohmann::json& response){
        
    }
};

#endif