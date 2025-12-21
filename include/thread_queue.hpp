#ifndef NADI_THREAD_QUEUE_HPP
#define NADI_THREAD_QUEUE_HPP
#include "nadi/nadi.h"
#include <optional>

class nadi_thread_queue_t {

    public:
    void push(const routed_message msg){

    }
    std::optional<routed_message> pop(){
        return {};
    }

};



#endif