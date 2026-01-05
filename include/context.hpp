#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include "message_routing.hpp"
#include "node_management.hpp"
#include <nadi\message_validation.hpp>
#include <nadi\message_helpers.hpp>
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
    context_t(shared_node_state& routes, std::vector<nadi_thread_t>& threads):nodes_(nadi_library{}),routes_(routes),threads_(threads){}
    void load_abstract_nodes(std::string path){
        nodes_.load_abstract_nodes(path);
    }
    // void handle_message(const nadi_message* msg, unsigned channel){
    //     try{
    //         auto routing = shared_routing_.get();
    //         const nadi_message* msg = incoming_messages_.front();
    //         incoming_messages_.pop_front();
    //         auto dest = routing->destinations_from({msg->node,msg->channel});
    //         if(dest.size()>0){
    //             for(const auto d:dest){
    //                 if(d.instance == 0){
    //                     handle_context_messages(msg,d.channel);
    //                 }
    //                 else{
    //                     //TODO dispatch to node
    //                 }
    //             }
    //         }
    //         else {

    //         }
    //         if(msg->channel != 0xF000){
    //             //not messages on the graph component
    //             const auto& destinations = routing->destinations_from({msg->node, msg->channel});
    //             if(destinations.size() == 1){ 
    //                 //if there is only one then just forward
    //                 auto[instance,channel] = destinations[0];
    //                 nodes_.lib_from_instance(instance).send(const_cast<nadi_message *>(msg),instance,channel);
    //             }
    //             else {
    //                 //TODO implement multi dispatch
    //             }
    //         }
    //         else{

    //         }
    //     }
    //     catch (...){
    //         //TODO logg errors, maybe handle some stuff
    //     }
    // }
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
                            auto msg = nadi::helpers::heap_allocate_connect_confirm(&free_msg,id);
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
