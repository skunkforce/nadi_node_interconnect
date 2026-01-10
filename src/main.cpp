#include <string>
#include "context.hpp"
#include "CLI/CLI.hpp"
#include <nlohmann/json.hpp>
#include "nadicpp/node.hpp"
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
// TODO REfactor config startup kram in eine parse config oder so
// Ziel: in der main parse-config aufrufen. rückgabe ist das config struct (Main enthält kein config parsing merh)
// Magic strings sind dann auch Teil der Konfig Routine

// Nodes .dll directory: god script oder cli paramter?
// Generell: welche entscheidungen kommen wo rein? god script ist auf jeden Fall ein cli parameter (wo dieses liegt)

// Abhängigkeiten zwischen verschiedenen Konfigurationen
// Bsp. 8 Threads angegeben, keine möglichkeit einer node thread 9 zuzordnen
// an einer Stelle angeben: GOd script, da nicht alles CLI parameter sein kann

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
    std::string bootstrap_file = "bootstrap.json";// Godscript (bootstrap weil andere das auch so nennen, etwa vcpkg)
    app.add_option("--bootstrap", bootstrap_file, "Path to bootstrap JSON file")->default_val((getExecutableDir()/"bootstrap.json").string());
    CLI11_PARSE(app, argc, argv);

    auto bootstrap_json = parse_bootstrap(bootstrap_file);
    //TODO validate bootstrap
    int num_threads = bootstrap_json["config"]["number_of_threads"];// dieser Wert muss gesetzt sein! -> bessere Fehlermeldungn, defaults etc.
    std::string nodes_dir = "";
    if(bootstrap_json["config"].contains("nodes_dir")){ 
        nodes_dir = bootstrap_json["config"]["nodes_dir"];
    }

    nadi_threads_t threads(num_threads,getExecutableDir().string()+"/"+nodes_dir);

    auto messages = bootstrap_json["messages"]; // Godscript hat zwei teile
    // 1. config 2. messages -> konfigurationsnachrichten, die an nodes gehen. INitiale nachrichten, nciht die nachrichtenformate (einmalig)
    for (const auto& msg_json : messages) {
        handle_bootstrap_message(msg_json,threads);
    }

    while(1){}
    return 0;
}