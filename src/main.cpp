#include <format>
#include <string>
#include "CLI/CLI.hpp"
#include <nlohmann/json.hpp>
#include "nadi/node.hpp"
#include <print>
#include "context.hpp"
#include <expected>
#include "bootstrap.hpp"
#include "thread.hpp"
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <limits.h>
#include <unistd.h>
#include "threads.hpp"
#endif

std::filesystem::path getExecutableDir()
{
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#elif defined(__linux__)
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        result[count] = '\0';
        return std::filesystem::path(result).parent_path();
    }
#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return std::filesystem::path(path).parent_path();
    }
#endif
    return std::filesystem::current_path();  // fallback
}


int main(int argc, char **argv) {
    CLI::App app{"Nadi Interconnect"};
    std::string bootstrap_file = "bootstrap.json";
    app.add_option("--bootstrap", bootstrap_file, "Path to bootstrap JSON file")->default_val((getExecutableDir()/"bootstrap.json").string());
    CLI11_PARSE(app, argc, argv);

    auto bootstrap_json = parse_bootstrap(bootstrap_file);
    //TODO validate bootstrap
    int num_threads = bootstrap_json["config"]["number_of_threads"];
    std::string nodes_dir = "";
    if(bootstrap_json["config"].contains("nodes_dir")){ 
        nodes_dir = bootstrap_json["config"]["nodes_dir"];
    }

    nadi_threads_t threads(num_threads,getExecutableDir().string()+"/"+nodes_dir);

    auto messages = bootstrap_json["messages"];
    for (const auto& msg_json : messages) {
        handle_bootstrap_message(msg_json,threads);
    }

    while(1){}
    return 0;
}