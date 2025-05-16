#ifndef FIND_NODES_HPP
#define FIND_NODES_HPP
#include <filesystem>
#include <iostream>
#include <format>




std::vector<std::filesystem::path> get_node_paths(const std::string& directory) {
    namespace fs = std::filesystem;
    std::vector<std::filesystem::path> out;
    try {
        // Iterate over directory entries
        for (const auto& entry : fs::directory_iterator(directory)) {
            // Check if the entry is a regular file and ends with .dll
            if (entry.is_regular_file() && entry.path().extension() == ".dll") {
                out.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << std::format("Error accessing directory '{}': {}\n", directory, e.what());
    }
    return out;
}
#endif