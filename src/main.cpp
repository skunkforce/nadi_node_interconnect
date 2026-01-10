#include <string>
#include "context.hpp"
#include <nlohmann/json.hpp>
#include "nadicpp/node.hpp"
#include "thread.hpp"
#include "system_specific.hpp"
#include "config.hpp"


int main(int argc, char **argv) {
    auto config = get_config(argc,argv);
    nadi_threads_t threads(config["config"]["number_of_threads"],config["config"]["nodes_path"]);

    for (const auto& msg_json : config["messages"]) {
        handle_bootstrap_message(msg_json,threads);
    }

    while(1){}
    return 0;
}