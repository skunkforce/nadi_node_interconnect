#ifndef NADI_THREAD_HPP
#define NADI_THREAD_HPP

#include "message_routing.hpp"
#include <thread>
#include <memory>
#include <deque>
#include "thread_queue.hpp"
#include <nlohmann/json.hpp>
class context_t;
class nadi_thread_t;
class nadi_threads_t;

class nadi_thread_t{
    std::deque<routed_message> local_queue_;
    std::unique_ptr<context_t> context_;
    const shared_node_state& routes_;
    nadi_thread_queue_t foreign_queue_;
    std::vector<nadi_thread_t>& threads_;
    const unsigned index_;
    friend void bootstrap_dispatch_to_target(const nadi_message* msg, const nlohmann::json& target, std::vector<nadi_thread_t>& threads, const nodes_routing& routes);
    friend class nadi_threads_t;
    std::unique_ptr<std::jthread> thread_;

    void construct_node(const std::string& node_path, const std::string& instance);

    void handle_context_messages(const nadi_message* msg, unsigned channel);

    void handle_message(const nodes_routing& table,const routed_message& m){
        if(m.instance == 0){
            if(m.channel == 0xF000){
                
            } 
        }
        else{
            table.library_instance(m.instance).send(const_cast<nadi_message *>(m.message),m.instance,m.channel);
        }
    }

    void push_message_non_local(const routed_message & rm){
        foreign_queue_.push(rm);
    }
    public:
    explicit nadi_thread_t(const shared_node_state& routes, unsigned index, std::unique_ptr<context_t> ctx, std::vector<nadi_thread_t>& threads);

    void callback(const nadi_message * m){
        auto r = routes_.get();
        auto ds = r->destinations_from(to_route_address(*m));
        for(auto&d:ds){
            routed_message rm{d};
            rm.message = m;
            unsigned idx = r->get_thread_index(d);
            if(idx == index_){
                local_queue_.push_back(rm);
            }
            else{
                threads_[idx].push_message_non_local(rm);
            }
        }
    }

    nadi_thread_t(const nadi_thread_t&) = delete;
    nadi_thread_t& operator=(const nadi_thread_t&) = delete;

    nadi_thread_t(nadi_thread_t&&) noexcept = default;
    nadi_thread_t& operator=(nadi_thread_t&&) noexcept = default;

    ~nadi_thread_t(){
        if(thread_){
            thread_->request_stop();
            if(thread_->joinable()) {
                thread_->join();
            }
        }
    }
};


#endif