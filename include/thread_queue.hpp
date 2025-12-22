#ifndef NADI_THREAD_QUEUE_HPP
#define NADI_THREAD_QUEUE_HPP
#include "nadi/nadi.h"
#include <optional>
#include "message_routing.hpp"
#include <atomic>


struct queue_node_t {
  std::atomic<queue_node_t*> next;
  routed_message value; 
};

class queue_impl_t {
public:
  queue_impl_t() : stub(std::make_unique<queue_node_t>()), head(stub.get()), tail(stub.get()) {
    stub->next.store(nullptr);
  }

  void push(queue_node_t* node) {
    node->next.store(nullptr, std::memory_order_relaxed);
    queue_node_t* prev = tail.exchange(node, std::memory_order_acq_rel);  
    prev->next.store(node, std::memory_order_release);           
  }

  queue_node_t* pop() {
    queue_node_t* head_copy = head.load(std::memory_order_relaxed);
    queue_node_t* next = head_copy->next.load(std::memory_order_acquire);

    if (next != nullptr) {
      head.store(next, std::memory_order_relaxed);
      head_copy->value = std::move(next->value);
      return head_copy;
    }
    return nullptr;
  }

private:
  std::unique_ptr<queue_node_t> stub;
  std::atomic<queue_node_t*> head;
  std::atomic<queue_node_t*> tail;
};

struct nadi_thread_queue_impl{
    queue_node_t buf_[100*1024];
    queue_impl_t full_; 
    queue_impl_t empty_;
};

class nadi_thread_queue_t{
    std::unique_ptr<nadi_thread_queue_impl> impl_;

    public:
    nadi_thread_queue_t():impl_(std::make_unique<nadi_thread_queue_impl>()){
        for(auto& e: impl_->buf_){
            impl_->empty_.push(&e);
        }
    }
    void push(routed_message msg){
        auto e = impl_->empty_.pop();
        e->value = std::move(msg);
        impl_->full_.push(e);
    }
    std::optional<routed_message> pop(){
        auto e = impl_->full_.pop();
        if(e){
            auto& msg = e->value;
            impl_->empty_.push(e);
            return std::move(msg);
        }
        return {};
    }

};



#endif