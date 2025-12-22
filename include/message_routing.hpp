#ifndef MESSAGE_ROUTING_HPP
#define MESSAGE_ROUTING_HPP
#include <nadi/nadi.h>
#include <algorithm>
#include <ranges>
#include <vector>
#include <atomic>
#include <memory>
#include <map>
#include <string>
#include "nadi/nadi.h"
#include "nadi/node.hpp"
#include "nadi/unique_message.hpp"

struct route_address{
    nadi_node_handle instance;
    unsigned int channel;
};

inline route_address to_route_address(const nadi_message& m){
    return {m.node,m.channel};
}

inline bool operator==(const route_address& lhs, const route_address& rhs){
    return lhs.instance == rhs.instance && lhs.channel == rhs.channel;
}

struct routed_message: public route_address{
    nadi_unique_message message;
    routed_message(routed_message&&) noexcept = default;
    routed_message& operator=(routed_message&&) noexcept = default;
    routed_message(const routed_message&) = delete;
    routed_message& operator=(const routed_message&) = delete;

    explicit routed_message(nadi_unique_message&& msg) noexcept
        : message(std::move(msg))
    {}
    template<typename... Args>
    routed_message(nadi_unique_message&& msg, Args&&... args)
        : route_address(std::forward<Args>(args)...),
          message(std::move(msg))
    {}
    routed_message():message(){}
};


class message_routing;
class nodes_routing{
    friend class message_routing;
    struct routes {
        route_address source_;
        std::vector<route_address> destinations_;
    };
    std::vector<routes> connections_;
    std::vector<std::pair<nadi_node_handle,unsigned>> thread_indexes_;
    std::map<std::string,nadi_node_handle> instance_map_;
    std::map<nadi_node_handle,nadi_library> library_map_;

    const std::vector<route_address> empty_;
    public:
    std::optional<std::vector<routes>::iterator> it_from_route_address(route_address a){
        auto it = std::ranges::find_if(connections_,[&](const routes& e){return e.source_ == a;});
        if(it != connections_.end()){
            return it;
        }
        return {};
    }

    std::optional<std::vector<routes>::const_iterator> it_from_route_address(route_address a) const{
        auto it = std::ranges::find_if(connections_,[&](const routes& e){return e.source_ == a;});
        if(it != connections_.end()){
            return it;
        }
        return {};
    }
    
    const std::vector<route_address>& destinations_from(route_address a ) const {
        if(auto it = it_from_route_address(a)){
            return (*it)->destinations_;
        }
        return empty_; 
    }
    unsigned get_thread_index(const nadi_node_handle h) const {
        auto it = std::ranges::find_if(thread_indexes_,[=](auto e){ return e.first == h;}); //TODO make this a sorted list so it goes faster
        if(it == thread_indexes_.end()){
            throw "index does not exist";
        }
        return it->second;
    }
    unsigned get_thread_index(const route_address& r) const{
        return get_thread_index(r.instance);
    }
    nadi_node_handle instance_name_to_handle(const std::string & instance) const {
        return instance_map_.at(instance);
    }
    nadi_library library_instance(nadi_node_handle h) const {
        return library_map_.at(h);
    }
    void add_node(const std::string& instance, const nadi_node_handle h, nadi_library lib, unsigned index){
        thread_indexes_.push_back({h,index}); //TODO make this a sorted list so it goes faster
        instance_map_[instance] = h;
        library_map_[h] = lib;
    }
};

class message_routing : public nodes_routing {

public:
    message_routing(const nodes_routing& source):nodes_routing{source}{}
    void connect(const nadi_node_handle source_instance, unsigned int source_channel,
                 const nadi_node_handle target_instance, unsigned int target_channel) {

        if(auto it = it_from_route_address(route_address{source_instance,source_channel})){
            auto& dest = (*it)->destinations_;
            route_address target{target_instance, target_channel};
            if(!std::ranges::contains(dest,target)){  //only insert once
                dest.push_back(target);
            }
        }
        else{
            connections_.emplace_back(route_address{source_instance,source_channel},std::vector<route_address>{route_address{target_instance,target_channel}});
        }
        // TODO: Send confirmation response
    }
    void disconnect(const nadi_node_handle source_instance, unsigned int source_channel,
                    const nadi_node_handle target_instance, unsigned int target_channel) {
        // Remove connection from connections
        // TODO: Send confirmation response
    }
};

class shared_node_state {
    std::atomic<std::shared_ptr<nodes_routing>> table_;

    public:
    shared_node_state():table_(std::make_shared<nodes_routing>()){}
    std::shared_ptr<const nodes_routing> get() const {
        return table_.load(std::memory_order_acquire);
    }
    void modify(auto f){
        std::shared_ptr<nodes_routing> local = table_.load(std::memory_order_acquire);
        auto new_routes = std::make_shared<message_routing>(*local);
        f(*new_routes);
        table_.store(new_routes, std::memory_order_release);
    }
};

#endif