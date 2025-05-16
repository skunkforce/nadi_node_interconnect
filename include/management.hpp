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
                auto it = std::ranges::find_if(routing_.connections,[&](const message_routing::route& e){return e.source == msg->instance;});
                if(it != routing_.connections.end()){
                    //found a connection from this source
                    auto cit = std::ranges::find_if(it->channels,[&](const auto& e){ return std::get<0>(e) == msg->channel; });
                    if(cit != it->channels.end()){
                        //found a connection from this source and channel
                        auto& destinations = std::get<1>(*cit);
                        if(destinations.size() == 1){ 
                            msg_guard.dismiss(); //will be recycled by the consumer
                            //if there is only one then just forward
                            auto[instance,lib] = destinations[0];
                            lib.send(msg,instance);
                        }
                        else {
                            msg_guard.dismiss(); //TODO implement reference counting to free at the right time.
                            //TODO implement multi dispatch
                        }
                    }
                }
            }
            else{
                //user input should be validated earlier, for example in the node which received user input from the outside
                auto json_meta = nlohmann::json::parse(msg->meta);
                auto json_data = nlohmann::json::parse(msg->data);
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
