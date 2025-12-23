#include "thread.hpp"
#include "context.hpp"

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

    void nadi_thread_t::on_callback(nadi_unique_message m){
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

    nadi_thread_t::~nadi_thread_t(){
        if(impl_){
            impl_->thread_.request_stop();
            if(impl_->thread_.joinable()) {
                impl_->thread_.join();
            }
        }
    }

    nadi_thread_t::nadi_thread_t(nadi_thread_t&&) noexcept = default;
    nadi_thread_t& nadi_thread_t::operator=(nadi_thread_t&&) noexcept = default;

    void nadi_thread_t::push_message_non_local(routed_message rm){
        impl_->foreign_queue_.push(std::move(rm));
    }

void callback(nadi_message* msg, void* thread_ctx);

    void nadi_thread_impl::construct_node(const std::string& node_path, const std::string& instance_name){        
        nadi_library lib = load_node(node_path); // Adjust extension
        if (lib.dll) {
            nadi_node_handle node_instance;
            lib.init(&node_instance, callback, &threads_[index_]);

            routes_.modify([&](auto& routes){
                routes.add_node(instance_name,node_instance,lib,index_);
            });
            local_nodes_.emplace_back(node_instance,lib);
        }
    }
    void nadi_thread_impl::handle_context_messages(nadi_unique_message msg, unsigned channel){
        if(msg.is_json_format()){
            if(auto json_msg = msg.to_json()){
                auto data = (*json_msg)["data"];
                std::string id = "";
                if(data.contains("id")){
                    id = data["id"];
                }
                if(channel == 0xF000){
                    if(nadi::validation::validate_context_node_create(data)){
                        std::string node_name = data["abstract_name"].get<std::string>();
                        std::string instance_name = data["instance_name"].get<std::string>();
                        std::string node_path = nodes_dir_ + node_name + ".dll";
                        construct_node(node_path,instance_name);
                    }
                    else if(nadi::validation::validate_context_node_destroy(data)){
                        
                    }
                    else if(nadi::validation::validate_context_abstract_nodes(data)){
                        auto list = nlohmann::json::array(); //TODO
                        auto msg = nadi::helpers::heap_allocate_abstract_nodes_list(list,&free_msg,id);                        
                    }
                    else if(nadi::validation::validate_context_nodes(data)){
                        
                    }
                }
                else if(channel == 0xF100){
                    if(context_){
                        (*context_).handle_context_messages(std::move(msg),channel);
                    }
                }
            }
        }
    }


    nadi_thread_t::nadi_thread_t(shared_node_state& routes, unsigned index, std::unique_ptr<context_t> ctx, std::vector<nadi_thread_t>& threads,const std::string& nodes_dir):impl_(std::make_unique<nadi_thread_impl>(std::deque<routed_message>{},std::move(ctx),routes,nadi_thread_queue_t{},threads,nodes_dir,index)){
        impl_->thread_ = 
            std::jthread([self = impl_.get()](std::stop_token stoken){
                auto& fq = self->foreign_queue_;
                while(!stoken.stop_requested()){
                    auto table = self->routes_.get();
                    if(auto rmsg = fq.pop(); rmsg){
                        self->handle_message(*table,std::move(*rmsg));
                    }
                    if(!self->local_queue_.empty()){
                        auto rmsg = std::move(self->local_queue_.front());
                        self->local_queue_.pop_front();
                        self->handle_message(*table,std::move(rmsg));
                    }
                    for(auto& n:self->local_nodes_){
                        n.second.handle_events(n.first);
                    }
                }
            });
    }

extern "C" {
    void callback(nadi_message* msg, void* thread_ctx){
        static_cast<nadi_thread_t*>(thread_ctx)->on_callback(nadi_unique_message(msg));
    }

    void free_msg(nadi_message* msg){
        //context.free(msg);
    }

    void free_function_msg(nadi_message *){
        
    }
}

