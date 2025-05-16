#ifndef MESSAGE_ROUTING_HPP
#define MESSAGE_ROUTING_HPP

class message_routing{
    public:
    struct route {
        void* source;
        //destinations [channel,[destinations...]]
        std::vector<std::tuple<unsigned int,std::vector<std::tuple<void*,nadi_library>>>> channels;
    };
    
    std::vector<route> connections;
};

#endif