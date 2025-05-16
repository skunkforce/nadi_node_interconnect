#ifndef MESSAGE_ROUTING_HPP
#define MESSAGE_ROUTING_HPP
#include "nadi.h"

struct route_address{
    nadi_instance_handle instance;
    unsigned int channel;
};

bool operator==(const route_address& lhs, const route_address& rhs){
    return lhs.instance == rhs.instance && lhs.channel == rhs.channel;
}

class message_routing {
    struct routes {
        route_address source_;
        std::vector<route_address> destinations_;
    };
public:
    void connect(const std::string& source_instance, unsigned int source_channel,
                 const std::string& target_instance, unsigned int target_channel) {
        // Update connections (e.g., add to connections vector)
        // TODO: Send confirmation response
    }
    void disconnect(const std::string& source_instance, unsigned int source_channel,
                    const std::string& target_instance, unsigned int target_channel) {
        // Remove connection from connections
        // TODO: Send confirmation response
    }
    void send_connections_list(nadi_instance_handle source_instance) {
        nlohmann::json connections = nlohmann::json::array();
        // Populate connections from internal routing data
        for (const auto& route : connections_) {
            // connections.push_back({
            //     {"source", {route.source_instance, route.source_channel}},
            //     {"target", {route.target_instance, route.target_channel}}
            // });
            //TODO get actual soource and dest from map of name to nadi_instance_handle
        }
        nlohmann::json response = {
            {"meta", {{"format", "json"}}},
            {"data", {
                {"type", "nodes.instances.connections.list"},
                {"connections", connections}
            }}
        };
        send_response(source_instance, response);
    }
    const std::vector<route_address>& destinations_from(route_address a ){
        auto it = std::ranges::find_if(connections_,[&](const routes& e){return e.source_ == a;});
        if(it != connections_.end()){
            return it->destinations_;
        }
    }
private:
    std::vector<routes> connections_;
    void send_response(nadi_instance_handle instance, const nlohmann::json& response){}
};

#endif