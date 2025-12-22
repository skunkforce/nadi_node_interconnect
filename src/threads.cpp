#include "threads.hpp"
#include "context.hpp"



nadi_threads_t::nadi_threads_t(unsigned num_threads, const std::string& nodes_dir){
    auto context = std::make_unique<context_t>(shared_routing,threads_);
    context->load_abstract_nodes(nodes_dir);
    threads_.emplace_back(shared_routing,0,std::move(context),threads_, nodes_dir);
    for(int i = 1;i<num_threads;i++){
        threads_.emplace_back(shared_routing,i,nullptr,threads_, nodes_dir);
    }
}