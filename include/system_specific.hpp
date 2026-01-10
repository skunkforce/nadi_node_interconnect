#ifndef NADI_INTERCONNECT_SYSTEM_SPECIFIC_HPP
#define NADI_INTERCONNECT_SYSTEM_SPECIFIC_HPP

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <limits.h>
#include <unistd.h>
#include "threads.hpp"
#endif

inline std::filesystem::path getExecutableDir()
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

#endif