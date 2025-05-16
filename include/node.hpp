#include "nadi.h"

// Platform-specific headers
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// Function pointer types for DLL functions
using nadi_init_pt = nadi_status (*)(nadi_instance_handle* instance, nadi_receive_callback);
using nadi_deinit_pt = nadi_status (*)(nadi_instance_handle instance);
using nadi_send_pt = nadi_status (*)(nadi_message* message, nadi_instance_handle instance);
using nadi_free_pt = void (*)(nadi_message*);
using nadi_descriptor_pt = char* (*)();

struct nadi_library{
    nadi_init_pt init;
    nadi_deinit_pt deinit;
    nadi_send_pt send;
    nadi_free_pt free;
    nadi_descriptor_pt descriptor;
    #ifdef _WIN32
    HMODULE dll;
    #else
    void * dll;
    #endif
};

nadi_library load_node(std::string path) {
    nadi_library lib{};
#ifdef _WIN32
    // Load DLL on Windows
    lib.dll = LoadLibraryA(path.c_str());
    if (!lib.dll) {
        //TODO handle error
        return lib;
    }
#else
    // Load DLL on Linux/macOS
    lib.dll = dlopen(path.c_str(), RTLD_LAZY);
    if (!lib.dll) {
        //TODO handle error
        return false;
    }
#endif

    // Load function pointers
#ifdef _WIN32
    lib.init = reinterpret_cast<nadi_init_pt>(GetProcAddress(lib.dll, "nadi_init"));
    lib.deinit = reinterpret_cast<nadi_deinit_pt>(GetProcAddress(lib.dll, "nadi_deinit"));
    lib.send = reinterpret_cast<nadi_send_pt>(GetProcAddress(lib.dll, "nadi_send"));
    lib.free = reinterpret_cast<nadi_free_pt>(GetProcAddress(lib.dll, "nadi_free"));
    lib.descriptor = reinterpret_cast<nadi_descriptor_pt>(GetProcAddress(lib.dll, "nadi_descriptor"));
#else
    lib.init = reinterpret_cast<nadi_init_pt>(dlsym(lib.dll, "nadi_init"));
    lib.deinit = reinterpret_cast<nadi_deinit_pt>(dlsym(lib.dll, "nadi_deinit"));
    lib.send = reinterpret_cast<nadi_send_pt>(dlsym(lib.dll, "nadi_send"));
    lib.free = reinterpret_cast<nadi_free_pt>(dlsym(lib.dll, "nadi_free"));
    lib.descriptor = reinterpret_cast<nadi_descriptor_pt>(dlsym(lib.dll, "nadi_descriptor"));
#endif

    // Check if functions were loaded
    if (!lib.init || !lib.deinit || !lib.send || !lib.free || !lib.descriptor) {
#ifdef _WIN32
        //std::cerr << "Failed to load functions: " << GetLastError() << "\n";
#else
        //std::cerr << "Failed to load functions: " << dlerror() << "\n";
#endif
#ifdef _WIN32
        FreeLibrary(lib.dll);
#else
        dlclose(lib.dll);
#endif
        lib.dll = 0;
    }

    return lib;
}


