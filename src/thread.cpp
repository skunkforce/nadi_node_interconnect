#include "thread.hpp"
#include "context.hpp"


    void nadi_thread_t::handle_context_messages(const nadi_message* msg, unsigned channel){
        auto json_meta = nlohmann::json::parse(msg->meta);
        if(json_meta.contains("format") && json_meta["format"] == "json"){
            auto data = (const char*)msg->data;
            auto json_data = nlohmann::json::parse(std::string_view(data,data+msg->data_length));
            auto id = json_data["id"];
            if(channel == 0xF000){
                if(nadi::validation::validate_context_node_create(json_data)){
                    std::string node_name = json_data["abstract_name"].get<std::string>();
                    std::string instance_name = json_data["instance_name"].get<std::string>();
                    //std::string node_path = nodes_dir_ + node_name + ".dll";
                    //nodes_.construct_node(node_path,instance_name,nullptr);
                }
                else if(nadi::validation::validate_context_node_destroy(json_data)){
                    
                }
                else if(nadi::validation::validate_context_abstract_nodes(json_data)){
                    auto list = nlohmann::json::array(); //TODO
                    auto msg = nadi::helpers::heap_allocate_abstract_nodes_list(list,&free_msg,id);                        
                }
                else if(nadi::validation::validate_context_nodes(json_data)){
                    
                }
            }
            else if(channel == 0xF100){
                if(context_){
                    (*context_).handle_context_messages(msg,channel);
                }
            }

        }
    }

    nadi_thread_t::nadi_thread_t(const shared_node_state& routes, unsigned index, std::unique_ptr<context_t> ctx, std::vector<nadi_thread_t>& threads):thread_( //caller must ensure the the reference to routes lives longer than the thread
        std::make_unique<std::jthread>(
            std::jthread([self = this](std::stop_token stoken){
                auto& fq = self->foreign_queue_;
                while(!stoken.stop_requested()){
                    auto table = self->routes_.get();
                    if(auto rmsg = fq.pop(); rmsg){
                        self->handle_message(*table,*rmsg);
                    }
                    if(!self->local_queue_.empty()){
                        auto rmsg = self->local_queue_.front();
                        self->local_queue_.pop_front();
                        self->handle_message(*table,rmsg);
                    }
                }
            }))
        ),context_(std::move(ctx)),routes_(routes),threads_(threads),index_(index){}

extern "C" {
    void callback(nadi_message* msg, void* thread_ctx){
        static_cast<nadi_thread_t*>(thread_ctx)->callback(msg);
    }

    void free_msg(nadi_message* msg){
        //context.free(msg);
    }
}

