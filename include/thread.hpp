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

struct nadi_thread_impl;

class nadi_thread_t{
    std::unique_ptr<nadi_thread_impl> impl_;
    friend void bootstrap_dispatch_to_target(nadicpp::message msg, const nlohmann::json& target, std::vector<nadi_thread_t>& threads, const nodes_routing& routes);
    friend class nadi_threads_t;

    void push_message_non_local(routed_message rm);
    public:
    explicit nadi_thread_t(shared_node_state& routes, unsigned index, std::unique_ptr<context_t> ctx, std::vector<nadi_thread_t>& threads, const std::string& nodes_dir);

    void on_callback(nadicpp::message m);

    nadi_thread_t(const nadi_thread_t&) = delete;
    nadi_thread_t& operator=(const nadi_thread_t&) = delete;

    nadi_thread_t(nadi_thread_t&&) noexcept;
    nadi_thread_t& operator=(nadi_thread_t&&) noexcept;

    ~nadi_thread_t();
};


#endif