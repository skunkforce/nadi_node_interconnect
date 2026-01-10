#ifndef NADI_INTERCONNECT_CONFIG_HPP
#define NADI_INTERCONNECT_CONFIG_HPP
#include "CLI/CLI.hpp"
#include "bootstrap.hpp"
#include <nlohmann/json.hpp>

nlohmann::json get_config(int argc, char **argv){
    CLI::App app{"Nadi Interconnect"};
    std::string bootstrap_file = "bootstrap.json";
    app.add_option("--bootstrap", bootstrap_file, "Path to bootstrap JSON file")->default_val((getExecutableDir()/"bootstrap.json").string());
    CLI11_PARSE(app, argc, argv);
   
    return parse_bootstrap(bootstrap_file);
}


#endif