#ifndef NODE_MANAGEMENT_HPP
#define NODE_MANAGEMENT_HPP
#include <nlohmann/json.hpp>
#include <nadi/nadi.h>
#include <vector>
#include <tuple>
#include "find_nodes.hpp"
#include "core_callbacks.hpp"
#include "nadi/node.hpp"
#include <expected>

class node_management{
    std::map<std::string,std::tuple<nadi_library,std::filesystem::path>> abstract_nodes_;

    // void erase_node(nadi_node_handle node){
    //     for (auto it = instance_map_.begin(); it != instance_map_.end();) {
    //         if (it->second == node) {
    //             it = instance_map_.erase(it); // Erase and advance to next iterator
    //         } else {
    //             ++it; 
    //         }
    //         library_map_.erase(node);
    //     }

    // }
    public:
    node_management(nadi_library ctx){

    }

    void load_abstract_nodes(std::string nodes_dir){
        auto node_paths_ = get_node_paths(nodes_dir);
        for(const auto &path: node_paths_){
            auto lib = load_node(path.string());
            std::string name = path.filename().string();
            abstract_nodes_[name] = std::tuple<nadi_library,std::filesystem::path>{lib,path};
        }
    }

    nlohmann::json abstract_nodes_as_json(){
        auto ret = nlohmann::json::array();
        for(const auto& an: abstract_nodes_){
            auto o = nlohmann::json::object();
            std::size_t len = 1024*100;
            auto buffer = std::make_unique<char[]>(len);
            std::get<0>(an.second).descriptor(buffer.get(),&len);
            auto desc = std::string(buffer.get(),len);
            if (nlohmann::json::accept(desc)) {           // fast validation only
                auto descobj = nlohmann::json::parse(desc); // safe now
                o["description"] = std::move(descobj);
            }

            //TODO put data into the object
            ret.push_back(o);
        }
        return ret;
    }
};

#endif