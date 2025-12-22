#include <nadi/nadi.h>
#include <nadi/message_validation.hpp>
#include <nadi/message_helpers.hpp>
#include <optional>
#include <time.h>
#include "scope_guard.hpp"
#include <chrono>


void free_msg(nadi_message* message){
    delete[] message->meta;
    auto pd = (char*)message->data;
    delete[] pd;
    delete message;
}

class interface_t{
    //calee responsible for taking the messages in a lock free and wait free manor
    nadi_receive_callback out_;
    void* receive_ctx_;
    nadi_node_handle node_id_;
    decltype(std::chrono::steady_clock::now()) last_sent_;

    interface_t(nadi_receive_callback cb, void* cb_ctx):out_{cb},receive_ctx_{cb_ctx},last_sent_{std::chrono::steady_clock::now()}{}
    public:
    static nadi_node_handle make(nadi_receive_callback cb,void* cb_ctx){
        auto i = new interface_t{cb,cb_ctx};
        i->node_id_ = i;
        return i;
    }
    nadi_status send(nadi_message* msg, unsigned channel){
        return NADI_OK;
    }
    nadi_status handle_events(){
        using namespace std::chrono_literals;
        if (std::chrono::steady_clock::now() > last_sent_ + 5s) {
            last_sent_ += 1s;
            auto m = nadi::helpers::heap_allocate_json_message(node_id_,1,nlohmann::json::parse(
R"(
{
    "message": "hello world"
})"
                ),free_msg);
            out_(m,receive_ctx_);
        }

        return NADI_OK;
    }

};


extern "C" {
    DLL_EXPORT nadi_status nadi_init(nadi_node_handle* node, nadi_receive_callback cb, void* cb_ctx){
        *node = interface_t::make(cb,cb_ctx);
        return NADI_OK;
    }

    DLL_EXPORT nadi_status nadi_deinit(nadi_node_handle node){
        delete static_cast<interface_t*>(node);
        return NADI_OK;
    }

    DLL_EXPORT nadi_status nadi_send(nadi_message* message, nadi_node_handle node, unsigned int target_channel){
        static_cast<interface_t*>(node)->send(message, target_channel);
        return NADI_OK;
    }

    DLL_EXPORT nadi_status nadi_handle_events(nadi_node_handle node){
        return static_cast<interface_t*>(node)->handle_events();
    }

    DLL_EXPORT nadi_status nadi_descriptor(char * descriptor, size_t* length){
        const char ret[] = R"({"name":"omnai-nadi-driver"})";
        if(sizeof(ret) > *length){
            return NADI_BUFFER_TOO_SMALL;
        }
        *length = sizeof(ret);
        strcpy(descriptor,ret);
        return NADI_OK;
    }
}