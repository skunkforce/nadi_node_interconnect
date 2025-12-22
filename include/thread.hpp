#ifndef NADI_THREAD_HPP
#define NADI_THREAD_HPP

#include "message_routing.hpp"
#include <thread>
#include <memory>
#include <deque>
#include "thread_queue.hpp"
#include <nlohmann/json.hpp>
#include <functional>
class context_t;
class nadi_thread_t;
class nadi_threads_t;

struct nadi_thread_impl{
    std::deque<routed_message> local_queue_;
    std::unique_ptr<context_t> context_;
    shared_node_state& routes_;
    nadi_thread_queue_t foreign_queue_;
    std::vector<nadi_thread_t>& threads_;
    std::string nodes_dir_;
    const unsigned index_;
    std::vector<std::pair<nadi_node_handle,nadi_library>> local_nodes_;
    std::jthread thread_;

    void handle_context_messages(nadi_unique_message msg, unsigned channel);
    void construct_node(const std::string& node_path, const std::string& instance);

    void handle_rpc(routed_message m){
        //TODO more validation
        auto f = reinterpret_cast<std::function<void()>*>(m.message.get()->data);
        (*f)();
    }

    void handle_message(const nodes_routing& table, routed_message m){
        if(m.instance == 0){
            handle_context_messages(std::move(m.message),m.channel);
        }
        else if(m.instance == (void*)1){ //RPC
            handle_rpc(std::move(m));
        }
        else{
            table.library_instance(m.instance).send(m.message.release(),m.instance,m.channel);
        }
    }
};

class nadi_thread_t{
    std::unique_ptr<nadi_thread_impl> impl_;
    friend void bootstrap_dispatch_to_target(nadi_unique_message msg, const nlohmann::json& target, std::vector<nadi_thread_t>& threads, const nodes_routing& routes);
    friend class nadi_threads_t;





    void push_message_non_local(routed_message rm){
        impl_->foreign_queue_.push(std::move(rm));
    }
    public:
    explicit nadi_thread_t(shared_node_state& routes, unsigned index, std::unique_ptr<context_t> ctx, std::vector<nadi_thread_t>& threads, const std::string& nodes_dir);

    void on_callback(nadi_unique_message m){
        auto r = impl_->routes_.get();
        auto ds = r->destinations_from(to_route_address(*m.get()));
        for(auto&d:ds){
            routed_message rm(std::move(m),d);
            unsigned idx = r->get_thread_index(d);
            if(idx == impl_->index_){
                impl_->local_queue_.push_back(std::move(rm));
            }
            else{
                impl_->threads_[idx].push_message_non_local(std::move(rm));
            }
        }
    }

    nadi_thread_t(const nadi_thread_t&) = delete;
    nadi_thread_t& operator=(const nadi_thread_t&) = delete;

    nadi_thread_t(nadi_thread_t&&) noexcept = default;
    nadi_thread_t& operator=(nadi_thread_t&&) noexcept = default;

    ~nadi_thread_t(){
        if(impl_){
            impl_->thread_.request_stop();
            if(impl_->thread_.joinable()) {
                impl_->thread_.join();
            }
        }
    }
};


#endif