#ifndef MANAGEMENT_HPP
#define MANAGEMENT_HPP
#include "message_routing.hpp"
#include "node_management.hpp"
#include "scope_guard.hpp"

class management {
    node_management nodes_;
    message_routing routing_;
    public:
    void load_nodes(std::string path){
        nodes_.load_nodes(path);
    }
    void callback(nadi_message* msg){
        try{
            auto msg_guard = sg::make_scope_guard([msg](){
                msg->free(msg);
            });
            if(msg->channel != 0xF000){
                //not messages on the graph component
                const auto& destinations = routing_.destinations_from({msg->instance, msg->channel});
                if(destinations.size() == 1){ 
                    msg_guard.dismiss(); //will be recycled by the consumer
                    //if there is only one then just forward
                    auto[instance,channel] = destinations[0];
                    nodes_.lib_from_instance(instance).send(msg,instance);
                }
                else {
                    msg_guard.dismiss(); //TODO implement reference counting to free at the right time.
                    //TODO implement multi dispatch
                }
            }
            else{
            auto json_meta = nlohmann::json::parse(msg->meta);
            if(json_meta.contains("format") && json_meta["format"] == "json"){
                    auto json_data = nlohmann::json::parse(msg->data);
                    if (json_data["type"] == "nodes.instances.construct") {
                        std::string node_name = json_data["node_name"];
                        std::string instance = json_data["instance"];
                        nodes_.construct_node(node_name, instance, msg->instance);
                    } else if (json_data["type"] == "nodes.instances.destruct") {
                        std::string instance = json_data["instance"];
                        nodes_.destruct_node(instance, msg->instance);
                    } else if (json_data["type"] == "nodes.instances.connections.connect") {
                        std::string source_instance = json_data["source"][0];
                        unsigned int source_channel = json_data["source"][1];
                        std::string target_instance = json_data["target"][0];
                        unsigned int target_channel = json_data["target"][1];
                        routing_.connect(source_instance, source_channel, target_instance, target_channel);
                    } else if (json_data["type"] == "nodes.instances.connections.disconnect") {
                        std::string source_instance = json_data["source"][0];
                        unsigned int source_channel = json_data["source"][1];
                        std::string target_instance = json_data["target"][0];
                        unsigned int target_channel = json_data["target"][1];
                        routing_.disconnect(source_instance, source_channel, target_instance, target_channel);
                    } else if (json_data["type"] == "nodes.instances.connections") {
                        routing_.send_connections_list(msg->instance);
                    } else if (json_data["type"] == "nodes.loaded") {
                        nodes_.send_loaded_list(msg->instance);
                    } else if (json_data["type"] == "nodes.instances") {
                        nodes_.send_instances_list(msg->instance);
                    }
                }
            }
        }
        catch (...){
            //TODO logg errors, maybe handle some stuff
        }
    }
    nlohmann::json to_json(){
        return nodes_.to_json();
    }
};



#endif
