#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include "message_routing.hpp"
#include "node_management.hpp"
#include <nadicpp\message_validation.hpp>
#include <nadicpp\message_helpers.hpp>
#include <deque>
#include <vector>
#include "thread.hpp"
#include <nadicpp\message.hpp>
extern "C"{
    void free_msg(nadi_message* msg);
};

class context_t {
    node_management nodes_;
    shared_node_state& routes_;
    std::vector<nadi_thread_t>& threads_;
    public:
    context_t(shared_node_state& routes, std::vector<nadi_thread_t>& threads):nodes_(nadicpp::library{}),routes_(routes),threads_(threads){}
    void load_abstract_nodes(std::string path){
        nodes_.load_abstract_nodes(path);
    }
    nlohmann::json abstract_nodes_as_json(){
        return nodes_.abstract_nodes_as_json();
    }
    friend void bootstrap_dispatch_to_target(nadicpp::message msg, const nlohmann::json& target, context_t& ctx);
    void handle_context_messages(nadicpp::message msg, unsigned channel){
        if(msg.is_json_format()){
            if(auto json_msg = msg.to_json()){
                auto data = (*json_msg)["data"];
                std::string id = "";
                if(data.contains("id")){
                    id = data["id"];
                }
                if(channel == 0xF100){
                    if(nadi::validation::validate_context_connect(data)){
                        auto r = routes_.get();
                        
                        auto sid = r->instance_name_to_handle(data["source"][0]);
                        unsigned source_channel = data["source"][1];
                        auto did = r->instance_name_to_handle(data["destination"][0]);
                        unsigned destination_channel = data["destination"][1];
                        if(sid && did){
                            routes_.modify([&](auto& routes){
                                routes.connect(sid,source_channel,did,destination_channel);
                            });
                            auto msg = nadicpp::helpers::heap_allocate_connect_confirm(&free_msg,id);
                        }
                    }
                    else if(nadi::validation::validate_context_disconnect(data)){
                        
                    }
                    else if(nadi::validation::validate_context_connections(data)){

                    }
                    else if(nadi::validation::validate_context_nodes(data)){
                        
                    }
                }
            }
        }
    }
};



#endif
