#include "threads.hpp"
#include "context.hpp"



nadi_threads_t::nadi_threads_t(unsigned num_threads, const std::string& nodes_dir){
    auto context = std::make_unique<context_t>(threads_);
    context->load_abstract_nodes(nodes_dir);
    threads_.push_back(nadi_thread_t(shared_routing,0,std::move(context),threads_));
    for(int i = 1;i<num_threads;i++){
        threads_.push_back(nadi_thread_t(shared_routing,i,nullptr,threads_));
    }
}