#ifndef NADI_THREADS_HPP
#define NADI_THREADS_HPP
#include "thread.hpp"

extern "C"{
    extern void free_function_msg(nadi_message *);
}

class nadi_threads_t{
    shared_node_state shared_routing;
    std::vector<nadi_thread_t> threads_;

    public:
    nadi_threads_t(unsigned num_threads, const std::string& nodes_dir);

    void push_bootstrap_message(std::string instance_name,routed_message msg){
        unsigned idx = 0;
        msg.message.get()->node = 0;
        if(instance_name != "context"){
            auto r = shared_routing.get();
            auto h = r->instance_name_to_handle(instance_name);
            msg.node = h;
            idx = r->get_thread_index(h);       
        }

        threads_[idx].push_message_non_local(std::move(msg));
    }
    void push_bootstrap_rpc(std::function<void()> f){
        for(auto& t:threads_){
            auto msg = new nadi_message();
            msg->meta = "{\"format\":\"function\"}";
            msg->data = new char[sizeof(std::function<void()>)];
            msg->data_length = sizeof(std::function<void()>);
            new (msg->data) std::function<void()>(f);
            msg->free = free_function_msg;
            msg->user = nullptr;
            routed_message m{nadicpp::message(msg)};
            m.channel = 0;
            m.node = (void*)1;

            t.push_message_non_local(std::move(m));
        }
    }

    std::size_t size() const {
        return threads_.size();
    }

};


#endif