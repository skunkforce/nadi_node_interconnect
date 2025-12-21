#ifndef NADI_THREADS_HPP
#define NADI_THREADS_HPP
#include "thread.hpp"

class nadi_threads_t{
    shared_node_state shared_routing;
    std::vector<nadi_thread_t> threads_;

    public:
    nadi_threads_t(unsigned num_threads, const std::string& nodes_dir);

    void push_message_non_local(const unsigned idx,const routed_message& msg){
        threads_[idx].push_message_non_local(msg);
    }

};


#endif