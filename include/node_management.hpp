#ifndef NODE_MANAGEMENT_HPP
#define NODE_MANAGEMENT_HPP
#include <nlohmann/json.hpp>
#include <nadi/nadi.h>
#include <vector>
#include <tuple>
#include "find_nodes.hpp"
#include "core_callbacks.hpp"
#include "nadicpp/node.hpp"
#include <expected>

class node_management{
    std::map<std::string,std::tuple<nadicpp::library,std::filesystem::path>> abstract_nodes_;
    public:
    node_management(nadicpp::library ctx){

    }

    void load_abstract_nodes(std::string nodes_dir){
        auto node_paths_ = get_node_paths(nodes_dir);
        for(const auto &path: node_paths_){
            auto lib = nadicpp::load_node(path.string());
            std::string name = path.filename().string();
            abstract_nodes_[name] = std::tuple<nadicpp::library,std::filesystem::path>{lib,path};
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